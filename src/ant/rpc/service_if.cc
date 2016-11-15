#include "ant/rpc/service_if.h"

#include <memory>
#include <string>
#include <google/protobuf/descriptor.pb.h>

#include "ant/base/strings/substitute.h"

#include "ant/rpc/connection.h"
#include "ant/rpc/inbound_call.h"
#include "ant/rpc/rpc_context.h"
#include "ant/rpc/rpc_header.pb.h"
//#include "ant/util/flag_tags.h"

// TODO remove this once we have fully cluster-tested this.
// Despite being on by default, this is left in in case we discover
// any issues in 0.10.0, we'll have an easy workaround to disable the feature.
DEFINE_bool(enable_exactly_once, true, "Whether to enable exactly once semantics.");
//TAG_FLAG(enable_exactly_once, hidden);

using google::protobuf::Message;
using std::string;
using std::unique_ptr;
using strings::Substitute;

namespace ant {
namespace rpc {

ServiceIf::~ServiceIf() {
}

void ServiceIf::Shutdown() {
}

bool ServiceIf::SupportsFeature(uint32_t feature) const {
  return false;
}

bool ServiceIf::ParseParam(InboundCall *call, google::protobuf::Message *message) {
  Slice param(call->serialized_request());
  if (PREDICT_FALSE(!message->ParseFromArray(param.data(), param.size()))) {
    string err = Substitute("invalid parameter for call $0: missing fields: $1",
                            call->remote_method().ToString(),
                            message->InitializationErrorString().c_str());
    LOG(WARNING) << err;
    call->RespondFailure(ErrorStatusPB::ERROR_INVALID_REQUEST,
                         Status::InvalidArgument(err));
    return false;
  }
  return true;
}

void ServiceIf::RespondBadMethod(InboundCall *call) {
  Sockaddr local_addr, remote_addr;

  CHECK_OK(call->connection()->socket()->GetSocketAddress(&local_addr));
  CHECK_OK(call->connection()->socket()->GetPeerAddress(&remote_addr));
  string err = Substitute("Call on service $0 received at $1 from $2 with an "
                          "invalid method name: $3",
                          call->remote_method().service_name(),
                          local_addr.ToString(),
                          remote_addr.ToString(),
                          call->remote_method().method_name());
  LOG(WARNING) << err;
  call->RespondFailure(ErrorStatusPB::ERROR_NO_SUCH_METHOD,
                       Status::InvalidArgument(err));
}

GeneratedServiceIf::~GeneratedServiceIf() {
}


void GeneratedServiceIf::Handle(InboundCall *call) {
  const RpcMethodInfo* method_info = call->method_info();
  if (!method_info) {
    RespondBadMethod(call);
    return;
  }
  unique_ptr<Message> req(method_info->req_prototype->New());
  if (PREDICT_FALSE(!ParseParam(call, req.get()))) {
    return;
  }
  Message* resp = method_info->resp_prototype->New();

  bool track_result = call->header().has_request_id()
                      && method_info->track_result
                      && FLAGS_enable_exactly_once;
  RpcContext* ctx = new RpcContext(call,
                                   req.release(),
                                   resp,
                                   track_result ? result_tracker_ : nullptr);
  if (track_result) {
    RequestIdPB request_id(call->header().request_id());
    ResultTracker::RpcState state = ctx->result_tracker()->TrackRpc(
        call->header().request_id(),
        resp,
        ctx);
    switch (state) {
      case ResultTracker::NEW:
        method_info->func(ctx->request_pb(), resp, ctx);
        break;
      case ResultTracker::COMPLETED:
      case ResultTracker::IN_PROGRESS:
      case ResultTracker::STALE:
        // ResultTracker has already responded to the RPC and deleted
        // 'ctx'.
        return;
      default:
        LOG(FATAL) << "Unknown state: " << state;
    }
  } else {
    method_info->func(ctx->request_pb(), resp, ctx);
  }
}


RpcMethodInfo* GeneratedServiceIf::LookupMethod(const RemoteMethod& method) {
  DCHECK_EQ(method.service_name(), service_name());
  const auto& it = methods_by_name_.find(method.method_name());
  if (PREDICT_FALSE(it == methods_by_name_.end())) {
    return nullptr;
  }
  return it->second.get();
}


} // namespace rpc
} // namespace ant
