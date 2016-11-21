#include "ant/master/worker_descriptor.h"

#include <math.h>
#include <mutex>
#include <unordered_set>
#include <vector>

#include <gflags/gflags.h>

#include "ant/common/wire_protocol.h"
#include "ant/base/strings/substitute.h"
#include "ant/master/master.pb.h"
#include "ant/util/net/net_util.h"
#include "ant/util/net/sockaddr.h"

DEFINE_int32(worker_server_unresponsive_timeout_ms, 60 * 1000,
"The period of time that a Master can go without receiving a"
" heatbeat from a worker server before considering it unresponsive" 
". Unresponsive servers are not selected.");

using std::make_shared;
using std::shared_ptr;


namespace ant {
namespace master {

Status WorkerDescriptor::RegisterNew(const NodeInstancePB& instance,
		                     const ServerRegistrationPB& registration,
				     shared_ptr<WorkerDescriptor>* desc) {
  shared_ptr<WorkerDescriptor> ret(make_shared<WorkerDescriptor>(instance.permanent_uuid()));
  RETURN_NOT_OK(ret->Register(instance, registration));
  desc->swap(ret);
  return Status::OK();
}

WorkerDescriptor::WorkerDescriptor(std::string perm_id)
    : permanent_uuid_(std::move(perm_id)),
      latest_seqno_(-1),
      last_heartbeat_(MonoTime::Now()) {
}
  
WorkerDescriptor::~WorkerDescriptor() {
}

static bool HostPortPBsEqual(
		const google::protobuf::RepeatedPtrField<HostPortPB>& pb1,
		const google::protobuf::RepeatedPtrField<HostPortPB>& pb2) {
  if (pb1.size() != pb2.size()) {
    return false;
  }
  
  std::unordered_set<HostPort, 
	             HostPortHasher, 
		     HostPortEqualityPredicate> hostports1;
  std::unordered_set<HostPort, 
	             HostPortHasher, 
		     HostPortEqualityPredicate> hostports2;
  for (int i = 0; i < pb1.size(); ++i) {
    HostPort hp1;
    HostPort hp2;
    if (!HostPortFromPB(pb1.Get(i), &hp1).ok()) return false;
    if (!HostPortFromPB(pb2.Get(i), &hp2).ok()) return false;
    hostports1.insert(hp1);
    hostports2.insert(hp2);
  }
  return hostports1 == hostports2;
}

Status WorkerDescriptor::Register(const NodeInstancePB& instance,
		                  const ServerRegistrationPB& registration) {
  std::lock_guard<simple_spinlock> l(lock_);
  CHECK_EQ(instance.permanent_uuid(), permanent_uuid_);

  if (registration_ &&
      (!HostPortPBsEqual(registration_->rpc_addresses(),
			 registration.rpc_addresses()) ||
       !HostPortPBsEqual(registration_->http_addresses(),
	                 registration.http_addresses()))) {
    std::string msg = strings::Substitute(
		  "Worker server $0 is attempting to re-register with "  
		  "a different host/port."
		  "This is not currently supported, Old: {$1} New: {$2}",
		  instance.permanent_uuid(),
		  registration_->ShortDebugString(),
		  registration.ShortDebugString());
    LOG(ERROR) << msg;
    return Status::InvalidArgument(msg);
  }

  if (registration.rpc_addresses().empty() ||
      registration.http_addresses().empty()) {
    return Status::InvalidArgument(
      "invalid registration: must have at least one RPC and one HTTP address"
      , registration.ShortDebugString());
  }

  if (instance.instance_seqno() < latest_seqno_) {
    return Status::AlreadyPresent(
      strings::Substitute("Cannot register with sequence number $0:"
	                  "Already have a registration from sequence number "
			  "$1", instance.instance_seqno(),
			        latest_seqno_));
  } else if (instance.instance_seqno() == latest_seqno_) {
    LOG(INFO) << "Processing retry of Worker registration from "
	      << instance.ShortDebugString();
  }

  latest_seqno_ = instance.instance_seqno();
  registration_.reset(new ServerRegistrationPB(registration));
  //TODO(wqx):
  //
  return Status::OK();
}

void WorkerDescriptor::UpdateHeartbeatTime() {
  std::lock_guard<simple_spinlock> l(lock_);
  last_heartbeat_ = MonoTime::Now();
}

MonoDelta WorkerDescriptor::TimeSinceHeartbeat() const {
  MonoTime now(MonoTime::Now());
  std::lock_guard<simple_spinlock> l(lock_);
  return now - last_heartbeat_;
}

bool WorkerDescriptor::PresumedDead() const {
  return TimeSinceHeartbeat().ToMilliseconds() >= 
	  FLAGS_worker_server_unresponsive_timeout_ms;
}

int64_t WorkerDescriptor::latest_seqno() const {
  std::lock_guard<simple_spinlock> l(lock_);
  return latest_seqno_;
}

void WorkerDescriptor::GetRegistration(ServerRegistrationPB* reg) const {
  std::lock_guard<simple_spinlock> l(lock_);
  CHECK(registration_) << "No registration";
  CHECK_NOTNULL(reg)->CopyFrom(*registration_);
}

void WorkerDescriptor::GetNodeInstancePB(NodeInstancePB* instance_pb) const {
  std::lock_guard<simple_spinlock> l(lock_);
  instance_pb->set_permanent_uuid(permanent_uuid_);
  instance_pb->set_instance_seqno(latest_seqno_);
}

Status WorkerDescriptor::ResolveSockaddr(Sockaddr* addr) const {
  vector<HostPort> hostports;
  {
    std::lock_guard<simple_spinlock> l(lock_);
    for (const HostPortPB& addr : registration_->rpc_addresses()) {
     hostports.push_back(HostPort(addr.host(), addr.port()));
    }
  }

  HostPort last_hostport;
  vector<Sockaddr> addrs;
  for (const HostPort& hostport : hostports) {
    RETURN_NOT_OK(hostport.ResolveAddresses(&addrs));
    if (!addrs.empty()) {
      last_hostport = hostport;
      break;
    }
  }
  if (addrs.size() == 0) {
    return Status::NetworkError("Unable to find the TS address: ", 
		     registration_->DebugString());
  }

  if (addrs.size() > 1) {
    LOG(WARNING) << "TS address " << last_hostport.ToString()
     << " resolves to " << addrs.size() << " different addresses. Using "
     << addrs[0].ToString();
  }
  *addr = addrs[0];
  return Status::OK();
}


std::string WorkerDescriptor::ToString() const {
  std::lock_guard<simple_spinlock> l(lock_);
  const auto& addr = registration_->rpc_addresses(0);
  return strings::Substitute("$0 ($1:$2)", permanent_uuid_,
		  addr.host(),
		  addr.port());
}

} // namespace master
} // namespace ant
