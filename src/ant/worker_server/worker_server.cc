#include "ant/worker_server/worker_server.h"

#include <glog/logging.h>
#include <list>
#include <vector>

#include "ant/base/strings/substitute.h"
#include "ant/rpc/service_if.h"
#include "ant/server/rpc_server.h"
#include "ant/server/web_server.h"

#include "ant/worker_server/heartbeater.h"
//#include "ant/worker_server/worker_service.h"
//
//
#include "ant/util/net/net_util.h"
#include "ant/util/net/sockaddr.h"
#include "ant/util/status.h"

using ant::rpc::ServiceIf;
using std::shared_ptr;
using std::vector;

namespace ant {
namespace worker_server {

WorkerServer::WorkerServer(const WorkerServerOptions& opts)
  : ServerBase("WorkerServer", opts, "ant.workerserver"),
    initted_(false),
    fail_heartbeats_for_tests_(false),
    opts_(opts) {
}

WorkerServer::~WorkerServer() {
  Shutdown();
}

string WorkerServer::ToString() const {
  return "WorkerServer";
}

Status WorkerServer::ValidateMasterAddressResolution() const {
  for (const HostPort& master_addr : opts_.master_addresses) {
    RETURN_NOT_OK_PREPEND(master_addr.ResolveAddresses(NULL),
                          strings::Substitute(
                              "Couldn't resolve master service address '$0'",
                              master_addr.ToString()));
  }
  return Status::OK();
}

Status WorkerServer::Init() {
  CHECK(!initted_);

  RETURN_NOT_OK(ValidateMasterAddressResolution());

  RETURN_NOT_OK(ServerBase::Init());

  heartbeater_.reset(new Heartbeater(opts_, this));

  initted_ = true;
  return Status::OK();
}

Status WorkerServer::WaitInited() {
//  return tablet_manager_->WaitForAllBootstrapsToFinish();
  return Status::OK();
}

Status WorkerServer::Start() {
  CHECK(initted_);

  RETURN_NOT_OK(ServerBase::Start());

  RETURN_NOT_OK(heartbeater_->Start());

  google::FlushLogFiles(google::INFO); // Flush the startup messages.

  return Status::OK();
}

void WorkerServer::Shutdown() {
  LOG(INFO) << "WorkerServer shutting down...";

  if (initted_) {
    WARN_NOT_OK(heartbeater_->Stop(), "Failed to stop TS Heartbeat thread");
    ServerBase::Shutdown();
  }

  LOG(INFO) << "WorkerServer shut down complete. Bye!";
}
} // namespace worker_server

} // namespace ant
