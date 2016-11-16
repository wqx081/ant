#include "ant/rpc/inbound_call.h"

#include <glog/stl_logging.h>
#include <memory>

#include "ant/base/strings/substitute.h"
#include "ant/rpc/connection.h"
#include "ant/rpc/rpc_introspection.pb.h"
#include "ant/rpc/rpc_sidecar.h"
#include "ant/rpc/rpcz_store.h"
#include "ant/rpc/serialization.h"
#include "ant/rpc/service_if.h"
//#include "ant/util/debug/trace_event.h"
#include "ant/util/metrics.h"
#include "ant/util/trace.h"

using google::protobuf::FieldDescriptor;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::Message;
using google::protobuf::MessageLite;
using std::shared_ptr;
using std::vector;
using strings::Substitute;

namespace ant {
namespace rpc {

InboundCall::InboundCall(Connection* conn)
  : conn_(conn),
    sidecars_deleter_(&sidecars_),
    trace_(new Trace),
    method_info_(nullptr) {
  RecordCallReceived();
}

InboundCall::~InboundCall() {}

Status InboundCall::ParseFrom(gscoped_ptr<InboundTransfer> transfer) {
//TODO(wqx)
#if 0
  TRACE_EVENT_FLOW_BEGIN0("rpc", "InboundCall", this);
  TRACE_EVENT0("rpc", "InboundCall::ParseFrom");
#endif
  RETURN_NOT_OK(serialization::ParseMessage(transfer->data(), &header_, &serialized_request_));

  // Adopt the service/method info from the header as soon as it's available.
  if (PREDICT_FALSE(!header_.has_remote_method())) {
    return Status::Corruption("Non-connection context request header must specify remote_method");
  }
  if (PREDICT_FALSE(!header_.remote_method().IsInitialized())) {
    return Status::Corruption("remote_method in request header is not initialized",
                              header_.remote_method().InitializationErrorString());
  }
  remote_method_.FromPB(header_.remote_method());

  // Retain the buffer that we have a view into.
  transfer_.swap(transfer);
  return Status::OK();
}

void InboundCall::RespondSuccess(const MessageLite& response) {
////  TRACE_EVENT0("rpc", "InboundCall::RespondSuccess");
  Respond(response, true);
}

void InboundCall::RespondUnsupportedFeature(const vector<uint32_t>& unsupported_features) {
////  TRACE_EVENT0("rpc", "InboundCall::RespondUnsupportedFeature");
  ErrorStatusPB err;
  err.set_message("unsupported feature flags");
  err.set_code(ErrorStatusPB::ERROR_INVALID_REQUEST);
  for (uint32_t feature : unsupported_features) {
    err.add_unsupported_feature_flags(feature);
  }

  Respond(err, false);
}

void InboundCall::RespondFailure(ErrorStatusPB::RpcErrorCodePB error_code,
                                 const Status& status) {
////  TRACE_EVENT0("rpc", "InboundCall::RespondFailure");
  ErrorStatusPB err;
  err.set_message(status.ToString());
  err.set_code(error_code);

  Respond(err, false);
}

void InboundCall::RespondApplicationError(int error_ext_id, const std::string& message,
                                          const MessageLite& app_error_pb) {
  ErrorStatusPB err;
  ApplicationErrorToPB(error_ext_id, message, app_error_pb, &err);
  Respond(err, false);
}

void InboundCall::ApplicationErrorToPB(int error_ext_id, const std::string& message,
                                       const google::protobuf::MessageLite& app_error_pb,
                                       ErrorStatusPB* err) {
  err->set_message(message);
  const FieldDescriptor* app_error_field =
    err->GetReflection()->FindKnownExtensionByNumber(error_ext_id);
  if (app_error_field != nullptr) {
    err->GetReflection()->MutableMessage(err, app_error_field)->CheckTypeAndMergeFrom(app_error_pb);
  } else {
    LOG(DFATAL) << "Unable to find application error extension ID " << error_ext_id
                << " (message=" << message << ")";
  }
}

void InboundCall::Respond(const MessageLite& response,
                          bool is_success) {
////  TRACE_EVENT_FLOW_END0("rpc", "InboundCall", this);
  SerializeResponseBuffer(response, is_success);

////  TRACE_EVENT_ASYNC_END1("rpc", "InboundCall", this,
////                         "method", remote_method_.method_name());
////  TRACE_TO(trace_, "Queueing $0 response", is_success ? "success" : "failure");
  RecordHandlingCompleted();
  conn_->rpcz_store()->AddCall(this);
  conn_->QueueResponseForCall(gscoped_ptr<InboundCall>(this));
}

void InboundCall::SerializeResponseBuffer(const MessageLite& response,
                                          bool is_success) {
  if (PREDICT_FALSE(!response.IsInitialized())) {
    LOG(ERROR) << "Invalid RPC response for " << ToString()
               << ": protobuf missing required fields: "
               << response.InitializationErrorString();
    // Send it along anyway -- the client will also notice the missing fields
    // and produce an error on the other side, but this will at least
    // make it clear on both sides of the RPC connection what kind of error
    // happened.
  }

  uint32_t protobuf_msg_size = response.ByteSize();

  ResponseHeader resp_hdr;
  resp_hdr.set_call_id(header_.call_id());
  resp_hdr.set_is_error(!is_success);
  uint32_t absolute_sidecar_offset = protobuf_msg_size;
  for (RpcSidecar* car : sidecars_) {
    resp_hdr.add_sidecar_offsets(absolute_sidecar_offset);
    absolute_sidecar_offset += car->AsSlice().size();
  }

  int additional_size = absolute_sidecar_offset - protobuf_msg_size;
  serialization::SerializeMessage(response, &response_msg_buf_,
                                  additional_size, true);
  int main_msg_size = additional_size + response_msg_buf_.size();
  serialization::SerializeHeader(resp_hdr, main_msg_size,
                                 &response_hdr_buf_);
}

void InboundCall::SerializeResponseTo(vector<Slice>* slices) const {
////  TRACE_EVENT0("rpc", "InboundCall::SerializeResponseTo");
  CHECK_GT(response_hdr_buf_.size(), 0);
  CHECK_GT(response_msg_buf_.size(), 0);
  slices->reserve(slices->size() + 2 + sidecars_.size());
  slices->push_back(Slice(response_hdr_buf_));
  slices->push_back(Slice(response_msg_buf_));
  for (RpcSidecar* car : sidecars_) {
    slices->push_back(car->AsSlice());
  }
}

Status InboundCall::AddRpcSidecar(gscoped_ptr<RpcSidecar> car, int* idx) {
  // Check that the number of sidecars does not exceed the number of payload
  // slices that are free (two are used up by the header and main message
  // protobufs).
  if (sidecars_.size() + 2 > OutboundTransfer::kMaxPayloadSlices) {
    return Status::ServiceUnavailable("All available sidecars already used");
  }
  sidecars_.push_back(car.release());
  *idx = sidecars_.size() - 1;
  return Status::OK();
}

string InboundCall::ToString() const {
  if (header_.has_request_id()) {
    return Substitute("Call $0 from $1 (ReqId={client: $2, seq_no=$3, attempt_no=$4})",
                      remote_method_.ToString(),
                      conn_->remote().ToString(),
                      header_.request_id().client_id(),
                      header_.request_id().seq_no(),
                      header_.request_id().attempt_no());
  }
  return Substitute("Call $0 from $1 (request call id $2)",
                      remote_method_.ToString(),
                      conn_->remote().ToString(),
                      header_.call_id());
}

void InboundCall::DumpPB(const DumpRunningRpcsRequestPB& req,
                         RpcCallInProgressPB* resp) {
  resp->mutable_header()->CopyFrom(header_);
  if (req.include_traces() && trace_) {
    resp->set_trace_buffer(trace_->DumpToString());
  }
  resp->set_micros_elapsed((MonoTime::Now() - timing_.time_received)
                           .ToMicroseconds());
}

const UserCredentials& InboundCall::user_credentials() const {
  return conn_->user_credentials();
}

const Sockaddr& InboundCall::remote_address() const {
  return conn_->remote();
}

const scoped_refptr<Connection>& InboundCall::connection() const {
  return conn_;
}

Trace* InboundCall::trace() {
  return trace_.get();
}

void InboundCall::RecordCallReceived() {
////  TRACE_EVENT_ASYNC_BEGIN0("rpc", "InboundCall", this);
  DCHECK(!timing_.time_received.Initialized());  // Protect against multiple calls.
  timing_.time_received = MonoTime::Now();
}

void InboundCall::RecordHandlingStarted(scoped_refptr<Histogram> incoming_queue_time) {
  DCHECK(incoming_queue_time != nullptr);
  DCHECK(!timing_.time_handled.Initialized());  // Protect against multiple calls.
  timing_.time_handled = MonoTime::Now();
  incoming_queue_time->Increment(
      (timing_.time_handled - timing_.time_received).ToMicroseconds());
}

void InboundCall::RecordHandlingCompleted() {
  DCHECK(!timing_.time_completed.Initialized());  // Protect against multiple calls.
  timing_.time_completed = MonoTime::Now();

  if (!timing_.time_handled.Initialized()) {
    // Sometimes we respond to a call before we begin handling it (e.g. due to queue
    // overflow, etc). These cases should not be counted against the histogram.
    return;
  }

  if (method_info_) {
    method_info_->handler_latency_histogram->Increment(
        (timing_.time_completed - timing_.time_handled).ToMicroseconds());
  }
}

bool InboundCall::ClientTimedOut() const {
  if (!header_.has_timeout_millis() || header_.timeout_millis() == 0) {
    return false;
  }

  MonoTime now = MonoTime::Now();
  int total_time = (now - timing_.time_received).ToMilliseconds();
  return total_time > header_.timeout_millis();
}

MonoTime InboundCall::GetClientDeadline() const {
  if (!header_.has_timeout_millis() || header_.timeout_millis() == 0) {
    return MonoTime::Max();
  }
  return timing_.time_received + MonoDelta::FromMilliseconds(header_.timeout_millis());
}

MonoTime InboundCall::GetTimeReceived() const {
  return timing_.time_received;
}

vector<uint32_t> InboundCall::GetRequiredFeatures() const {
  vector<uint32_t> features;
  for (uint32_t feature : header_.required_feature_flags()) {
    features.push_back(feature);
  }
  return features;
}

} // namespace rpc
} // namespace kudu
