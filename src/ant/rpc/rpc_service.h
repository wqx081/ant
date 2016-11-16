#ifndef KUDU_RPC_SERVICE_H_
#define KUDU_RPC_SERVICE_H_

#include "ant/base/ref_counted.h"
#include "ant/util/status.h"

namespace ant {
namespace rpc {

class RemoteMethod;
struct RpcMethodInfo;
class InboundCall;

class RpcService : public base::RefCountedThreadSafe<RpcService> {
 public:
  virtual ~RpcService() {}

  // Enqueue a call for processing.
  // On failure, the RpcService::QueueInboundCall() implementation is
  // responsible for responding to the client with a failure message.
  virtual Status QueueInboundCall(gscoped_ptr<InboundCall> call) = 0;

  virtual RpcMethodInfo* LookupMethod(const RemoteMethod& method) {
    return nullptr;
  }
};

} // namespace rpc
} // namespace ant

#endif // KUDU_RPC_SERVICE_H_
