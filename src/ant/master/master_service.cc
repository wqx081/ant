#include "ant/master/master_service.h"

#include <gflags/gflags.h>
#include <memory>
#include <string>
#include <vector>

#include "ant/common/wire_protocol.h"
#include "ant/master/master.h"
#include "ant/master/worker_manager.h"
#include "ant/master/worker_descriptor.h"
#include "ant/rpc/rpc_context.h"
#include "ant/server/web_server.h"
#include "ant/base/strings/substitute.h"

namespace ant {
namespace master {

using std::string;
using std::vector;
using std::shared_ptr;
using strings::Substitute;

MasterServiceImpl::MasterServiceImpl(Master* server)
  : MasterServiceIf(server->metric_entity(), server->result_tracker()),
    server_(server) {
}

void MasterServiceImpl::Ping(const PingRequestPB* req,
                             PingResponsePB* resp,
                             rpc::RpcContext* rpc) {
  LOG(INFO) << "RP: Ping";
  rpc->RespondSuccess();
}


void MasterServiceImpl::WorkerHeartbeat(const WorkerHeartbeatRequestPB* request,
                                        WorkerHeartbeatResponsePB* response,
                                        rpc::RpcContext* rpc) {
  //bool is_leader_master
  response->mutable_master_instance()->CopyFrom(server_->instance_pb());

  shared_ptr<WorkerDescriptor> worker_desc;
  if (request->has_registration()) {
    Status s = server_->worker_manager()->RegisterWorker(request->common().worker_instance(),
                                                         request->registration(),
                                                         &worker_desc);
    if (!s.ok()) {
      LOG(WARNING) << Substitute("Unable to register tserver ($0): $1",
                                 rpc->requestor_string(), s.ToString());
      // TODO: add service-specific errors
      rpc->RespondFailure(s);
      return;
    }
  } else {
    Status s = server_->worker_manager()->LookupWorker(request->common().worker_instance(), 
                                                       &worker_desc);
    if (s.IsNotFound()) {
     LOG(INFO) << Substitute("Got heartbeat from unknown tserver ($0) as $1; "
          "Asking this server to re-register.",
           request->common().worker_instance().ShortDebugString(), rpc->requestor_string());
      response->set_needs_reregister(true);

      rpc->RespondSuccess();
      return;
    }
  }

  worker_desc->UpdateHeartbeatTime();

  // TODO(wqx)
  // Only leaders handle worker reports
  LOG(INFO) << "----- Handle Worker Report";
  rpc->RespondSuccess();
}

} // namespace master
} // namespace ant
