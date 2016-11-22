#ifndef ANT_MASTER_MASTER_SERVICE_H_
#define ANT_MASTER_MASTER_SERVICE_H_
#include "ant/base/macros.h"
#include "ant/master/master.service.pb.h"
#include "ant/util/metrics.h"

namespace ant {

class NodeInstancePB;

namespace master {

class Master;
class WorkerDescriptor;

class MasterServiceImpl : public MasterServiceIf {
 public:
  explicit MasterServiceImpl(Master* server);
  virtual void Ping(const PingRequestPB* request,
                    PingResponsePB* response,
                    rpc::RpcContext* rpc) override;
  virtual void WorkerHeartbeat(const WorkerHeartbeatRequestPB* request,
                               WorkerHeartbeatResponsePB* response,
                               rpc::RpcContext* rpc) override;

 private:
  Master* server_;

  DISALLOW_COPY_AND_ASSIGN(MasterServiceImpl);
};

} // namespace master
} // namespace ant
#endif // ANT_MASTER_MASTER_SERVICE_H_
