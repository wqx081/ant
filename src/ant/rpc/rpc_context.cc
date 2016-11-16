#include "ant/rpc/rpc_context.h"

#include <ostream>
#include <sstream>

#include "ant/rpc/outbound_call.h"
#include "ant/rpc/inbound_call.h"
#include "ant/rpc/result_tracker.h"
#include "ant/rpc/rpc_sidecar.h"
#include "ant/rpc/service_if.h"
#include "ant/util/hdr_histogram.h"
#include "ant/util/metrics.h"
#include "ant/util/trace.h"
/// #include "ant/util/pb_util.h"

using google::protobuf::Message;

namespace ant {
namespace rpc {

RpcContext::RpcContext(InboundCall *call,
                       const google::protobuf::Message *request_pb,
                       google::protobuf::Message *response_pb,
                       const scoped_refptr<ResultTracker>& result_tracker)
  : call_(CHECK_NOTNULL(call)),
    request_pb_(request_pb),
    response_pb_(response_pb),
    result_tracker_(result_tracker) {
  VLOG(4) << call_->remote_method().service_name() << ": Received RPC request for "
          << call_->ToString() << ":" << std::endl << request_pb_->DebugString();
////  TRACE_EVENT_ASYNC_BEGIN2("rpc_call", "RPC", this,
////                           "call", call_->ToString(), "request", pb_util::PbTracer::TracePb(*request_pb_));
}

RpcContext::~RpcContext() {
}

void RpcContext::RespondSuccess() {
  if (AreResultsTracked()) {
    result_tracker_->RecordCompletionAndRespond(call_->header().request_id(),
                                                response_pb_.get());
  } else {
    VLOG(4) << call_->remote_method().service_name() << ": Sending RPC success response for "
        << call_->ToString() << ":" << std::endl << response_pb_->DebugString();
////    TRACE_EVENT_ASYNC_END2("rpc_call", "RPC", this,
////                           "response", pb_util::PbTracer::TracePb(*response_pb_),
////                           "trace", trace()->DumpToString());
    call_->RespondSuccess(*response_pb_);
    delete this;
  }
}

void RpcContext::RespondNoCache() {
  if (AreResultsTracked()) {
    result_tracker_->FailAndRespond(call_->header().request_id(),
                                    response_pb_.get());
  } else {
    VLOG(4) << call_->remote_method().service_name() << ": Sending RPC failure response for "
        << call_->ToString() << ": " << response_pb_->DebugString();
////    TRACE_EVENT_ASYNC_END2("rpc_call", "RPC", this,
////                           "response", pb_util::PbTracer::TracePb(*response_pb_),
////                           "trace", trace()->DumpToString());
    // This is a bit counter intuitive, but when we get the failure but set the error on the
    // call's response we call RespondSuccess() instead of RespondFailure().
    call_->RespondSuccess(*response_pb_);
    delete this;
  }
}

void RpcContext::RespondFailure(const Status &status) {
  if (AreResultsTracked()) {
    result_tracker_->FailAndRespond(call_->header().request_id(),
                                    ErrorStatusPB::ERROR_APPLICATION, status);
  } else {
    VLOG(4) << call_->remote_method().service_name() << ": Sending RPC failure response for "
        << call_->ToString() << ": " << status.ToString();
////    TRACE_EVENT_ASYNC_END2("rpc_call", "RPC", this,
////                           "status", status.ToString(),
////                           "trace", trace()->DumpToString());
    call_->RespondFailure(ErrorStatusPB::ERROR_APPLICATION, status);
    delete this;
  }
}

void RpcContext::RespondRpcFailure(ErrorStatusPB_RpcErrorCodePB err, const Status& status) {
  if (AreResultsTracked()) {
    result_tracker_->FailAndRespond(call_->header().request_id(),
                                    err, status);
  } else {
    VLOG(4) << call_->remote_method().service_name() << ": Sending RPC failure response for "
        << call_->ToString() << ": " << status.ToString();
////    TRACE_EVENT_ASYNC_END2("rpc_call", "RPC", this,
////                           "status", status.ToString(),
////                           "trace", trace()->DumpToString());
    call_->RespondFailure(err, status);
    delete this;
  }
}

void RpcContext::RespondApplicationError(int error_ext_id, const std::string& message,
                                         const Message& app_error_pb) {
  if (AreResultsTracked()) {
    result_tracker_->FailAndRespond(call_->header().request_id(),
                                    error_ext_id, message, app_error_pb);
  } else {
    if (VLOG_IS_ON(4)) {
      ErrorStatusPB err;
      InboundCall::ApplicationErrorToPB(error_ext_id, message, app_error_pb, &err);
      VLOG(4) << call_->remote_method().service_name()
          << ": Sending application error response for " << call_->ToString()
          << ":" << std::endl << err.DebugString();
    }
////    TRACE_EVENT_ASYNC_END2("rpc_call", "RPC", this,
////                           "response", pb_util::PbTracer::TracePb(app_error_pb),
////                           "trace", trace()->DumpToString());
    call_->RespondApplicationError(error_ext_id, message, app_error_pb);
    delete this;
  }
}

const rpc::RequestIdPB* RpcContext::request_id() const {
  return call_->header().has_request_id() ? &call_->header().request_id() : nullptr;
}

Status RpcContext::AddRpcSidecar(gscoped_ptr<RpcSidecar> car, int* idx) {
  return call_->AddRpcSidecar(std::move(car), idx);
}

const UserCredentials& RpcContext::user_credentials() const {
  return call_->user_credentials();
}

const Sockaddr& RpcContext::remote_address() const {
  return call_->remote_address();
}

std::string RpcContext::requestor_string() const {
  return call_->user_credentials().ToString() + " at " +
    call_->remote_address().ToString();
}

MonoTime RpcContext::GetClientDeadline() const {
  return call_->GetClientDeadline();
}

Trace* RpcContext::trace() {
  return call_->trace();
}

void RpcContext::Panic(const char* filepath, int line_number, const string& message) {
  // Use the LogMessage class directly so that the log messages appear to come from
  // the line of code which caused the panic, not this code.
#define MY_ERROR google::LogMessage(filepath, line_number, google::GLOG_ERROR).stream()
#define MY_FATAL google::LogMessageFatal(filepath, line_number).stream()

  MY_ERROR << "Panic handling " << call_->ToString() << ": " << message;
  MY_ERROR << "Request:\n" << request_pb_->DebugString();
  Trace* t = trace();
  if (t) {
    MY_ERROR << "RPC trace:";
    t->Dump(&MY_ERROR, true);
  }
  MY_FATAL << "Exiting due to panic.";

#undef MY_ERROR
#undef MY_FATAL
}


} // namespace rpc
} // namespace kudu
