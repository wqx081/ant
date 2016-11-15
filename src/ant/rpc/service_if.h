#ifndef KUDU_RPC_SERVICE_IF_H
#define KUDU_RPC_SERVICE_IF_H

#include <unordered_map>
#include <string>

#include "ant/base/macros.h"
#include "ant/base/ref_counted.h"
#include "ant/util/metrics.h"
#include "ant/util/net/sockaddr.h"
#include "ant/rpc/result_tracker.h"

namespace google {
namespace protobuf {
class Message;
}
}

namespace ant {

class Histogram;

namespace rpc {

class InboundCall;
class RemoteMethod;
class RpcContext;

// Generated services define an instance of this class for each
// method that they implement. The generic server code implemented
// by GeneratedServiceIf look up the RpcMethodInfo in order to handle
// each RPC.
struct RpcMethodInfo : public RefCountedThreadSafe<RpcMethodInfo> {
  // Prototype protobufs for requests and responses.
  // These are empty protobufs which are cloned in order to provide an
  // instance for each request.
  std::unique_ptr<google::protobuf::Message> req_prototype;
  std::unique_ptr<google::protobuf::Message> resp_prototype;

  scoped_refptr<Histogram> handler_latency_histogram;

  // Whether we should track this method's result, using ResultTracker.
  bool track_result;

  // The actual function to be called.
  std::function<void(const google::protobuf::Message* req,
                     google::protobuf::Message* resp,
                     RpcContext* ctx)> func;
};

// Handles incoming messages that initiate an RPC.
class ServiceIf {
 public:
  virtual ~ServiceIf();
  virtual void Handle(InboundCall* incoming) = 0;
  virtual void Shutdown();
  virtual std::string service_name() const = 0;

  // The service should return true if it supports the provided application
  // specific feature flag.
  virtual bool SupportsFeature(uint32_t feature) const;

  // Look up the method being requested by the remote call.
  //
  // If this returns nullptr, then certain functionality like
  // metrics collection will not be performed for this call.
  virtual RpcMethodInfo* LookupMethod(const RemoteMethod& method) {
    return nullptr;
  }

 protected:
  bool ParseParam(InboundCall* call, google::protobuf::Message* message);
  void RespondBadMethod(InboundCall* call);
};


// Base class for code-generated service classes.
class GeneratedServiceIf : public ServiceIf {
 public:
  virtual ~GeneratedServiceIf();

  // Looks up the appropriate method in 'methods_by_name_' and executes
  // it on the current thread.
  //
  // If no such method is found, responds with an error.
  void Handle(InboundCall* incoming) override;

  RpcMethodInfo* LookupMethod(const RemoteMethod& method) override;

 protected:
  // For each method, stores the relevant information about how to handle the
  // call. Methods are inserted by the constructor of the generated subclass.
  // After construction, this map is accessed by multiple threads and therefore
  // must not be modified.
  std::unordered_map<std::string, scoped_refptr<RpcMethodInfo>> methods_by_name_;

  // The result tracker for this service's methods.
  scoped_refptr<ResultTracker> result_tracker_;
};

} // namespace rpc
} // namespace ant
#endif
