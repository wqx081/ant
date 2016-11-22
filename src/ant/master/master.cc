#include "ant/master/master.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include <list>
#include <memory>
#include <vector>

#include "ant/common/wire_protocol.h"
#include "ant/base/strings/substitute.h"
#include "ant/master/master_service.h"
#include "ant/master/master.proxy.pb.h"
#include "ant/master/worker_manager.h"
#include "ant/rpc/messenger.h"
#include "ant/rpc/service_if.h"
#include "ant/rpc/service_pool.h"
#include "ant/server/rpc_server.h"

#include "ant/util/net/net_util.h"
#include "ant/util/net/sockaddr.h"
#include "ant/util/status.h"
#include "ant/util/threadpool.h"
#include "ant/common/version_info.h"

DEFINE_int32(master_registration_rpc_timeout_ms, 1500,
"Timeout for retrieving master registration over RPC.");

using std::min;
using std::shared_ptr;
using std::vector;

using ant::rpc::ServiceIf;
using strings::Substitute;

namespace ant {
namespace master {

Master::Master(const MasterOptions& opts)
  : ServerBase("Master", opts, "ant.master"),
    state_(kStopped),
    worker_manager_(new WorkerManager()),
    opts_(opts),
    registration_initialized_(false) {
}

Master::~Master() {
  CHECK_NE(kRunning, state_);
}

string Master::ToString() const {
  if (state_ != kRunning) {
    return "Master (stopped)";
  }
  return strings::Substitute("Master@$0", first_rpc_address().ToString());
}

Status Master::Init() {
  CHECK_EQ(kStopped, state_);


  RETURN_NOT_OK(ThreadPoolBuilder("init").set_max_threads(1).Build(&init_pool_));

  RETURN_NOT_OK(ServerBase::Init());

  state_ = kInitialized;
  return Status::OK();
}

Status Master::Start() {
  RETURN_NOT_OK(StartAsync());
////  RETURN_NOT_OK(WaitForCatalogManagerInit());
  google::FlushLogFiles(google::INFO); // Flush the startup messages.
  return Status::OK();
}

Status Master::StartAsync() {
  CHECK_EQ(kInitialized, state_);

//  RETURN_NOT_OK(maintenance_manager_->Init());

  gscoped_ptr<ServiceIf> impl(new MasterServiceImpl(this));
//  gscoped_ptr<ServiceIf> consensus_service(new ConsensusServiceImpl(
//      metric_entity(), result_tracker(), catalog_manager_.get()));
//  gscoped_ptr<ServiceIf> tablet_copy_service(new TabletCopyServiceImpl(
//      fs_manager_.get(), catalog_manager_.get(), metric_entity(), result_tracker()));

  RETURN_NOT_OK(ServerBase::RegisterService(std::move(impl)));
//  RETURN_NOT_OK(ServerBase::RegisterService(std::move(consensus_service)));
//  RETURN_NOT_OK(ServerBase::RegisterService(std::move(tablet_copy_service)));
  RETURN_NOT_OK(ServerBase::Start());

  RETURN_NOT_OK(InitMasterRegistration());

//  RETURN_NOT_OK(init_pool_->SubmitClosure(Bind(&Master::InitCatalogManagerTask,
//                                                Unretained(this))));

  state_ = kRunning;

  return Status::OK();
}

void Master::Shutdown() {
  if (state_ == kRunning) {
    string name = ToString();
    LOG(INFO) << name << " shutting down...";
//    maintenance_manager_->Shutdown();
    ServerBase::Shutdown();
//    catalog_manager_->Shutdown();
    LOG(INFO) << name << " shutdown complete.";
  }
  state_ = kStopped;
}

Status Master::GetMasterRegistration(ServerRegistrationPB* reg) const {
  if (!registration_initialized_.load(std::memory_order_acquire)) {
    return Status::ServiceUnavailable("Master startup not complete");
  }
  reg->CopyFrom(registration_);
  return Status::OK();
}

Status Master::InitMasterRegistration() {
  CHECK(!registration_initialized_.load());

  ServerRegistrationPB reg;
  vector<Sockaddr> rpc_addrs;
  RETURN_NOT_OK_PREPEND(rpc_server()->GetBoundAddresses(&rpc_addrs),
                                                "Couldn't get RPC addresses");
  RETURN_NOT_OK(AddHostPortPBs(rpc_addrs, reg.mutable_rpc_addresses()));
  vector<Sockaddr> http_addrs;
  web_server()->GetBoundAddresses(&http_addrs);
  RETURN_NOT_OK(AddHostPortPBs(http_addrs, reg.mutable_http_addresses()));
  reg.set_software_version(VersionInfo::GetShortVersionString());

  registration_.Swap(&reg);
  registration_initialized_.store(true);

  return Status::OK();
}

} // namespace master
} // namespace ant
