#ifndef ANT_MASTER_WORKER_DESCRIPTOR_H_
#define ANT_MASTER_WORKER_DESCRIPTOR_H_

#include <memory>
#include <mutex>
#include <string>

#include "ant/base/gscoped_ptr.h"
#include "ant/util/locks.h"
#include "ant/util/make_shared.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"

namespace ant {

class NodeInstancePB;
class Sockaddr;
class ServerRegistrationPB;

namespace rpc {
class Messenger;
} // namespace rpc

namespace master {

class WorkerDescriptor {
 public:
  static Status RegisterNew(const NodeInstancePB& instance,
		            const ServerRegistrationPB& registration,
			    std::shared_ptr<WorkerDescriptor>* desc);

  virtual ~WorkerDescriptor();

  void UpdateHeartbeatTime();
  MonoDelta TimeSinceHeartbeat() const;
  bool PresumedDead() const;
  Status Register(const NodeInstancePB& instance,
		  const ServerRegistrationPB& registration);
  const std::string& permanent_uuid() const;
  int64_t latest_seqno() const;
  void GetRegistration(ServerRegistrationPB* reg) const;
  void GetNodeInstancePB(NodeInstancePB* instance) const;

  std::string ToString() const;

 private:
  explicit WorkerDescriptor(std::string perm_id);

  Status ResolveSockaddr(Sockaddr* addr) const;
  mutable simple_spinlock lock_;

  const std::string permanent_uuid_;
  int64_t latest_seqno_;

  MonoTime last_heartbeat_;

  gscoped_ptr<ServerRegistrationPB> registration_;

  ALLOW_MAKE_SHARED(WorkerDescriptor);
  DISALLOW_COPY_AND_ASSIGN(WorkerDescriptor);  
};

} // namespace master
} // namespace ant
#endif // ANT_MASTER_WORKER_DESCRIPTOR_H_
