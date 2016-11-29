// THIS FILE IS AUTOGENERATED FROM ant/server/server_base.proto

#ifndef KUDU_RPC_SERVER_BASE_SERVICE_IF_DOT_H
#define KUDU_RPC_SERVER_BASE_SERVICE_IF_DOT_H

#include "ant/server/server_base.pb.h"

#include <functional>
#include <memory>
#include <string>

#include "ant/rpc/rpc_header.pb.h"
#include "ant/rpc/service_if.h"

namespace ant {
class MetricEntity;
namespace rpc {
class Messenger;
class ResultTracker;
class RpcContext;
} // namespace rpc
} // namespace ant

namespace ant {
namespace server {

class GenericServiceIf : public ::ant::rpc::GeneratedServiceIf {
 public:
  explicit GenericServiceIf(const scoped_refptr<MetricEntity>& entity, const scoped_refptr<rpc::ResultTracker>& result_tracker);
  virtual ~GenericServiceIf();
  std::string service_name() const override;
  static std::string static_service_name();

  virtual void SetFlag(const SetFlagRequestPB *req,
     SetFlagResponsePB *resp, ::ant::rpc::RpcContext *context) = 0;
  virtual void FlushCoverage(const FlushCoverageRequestPB *req,
     FlushCoverageResponsePB *resp, ::ant::rpc::RpcContext *context) = 0;
  virtual void ServerClock(const ServerClockRequestPB *req,
     ServerClockResponsePB *resp, ::ant::rpc::RpcContext *context) = 0;
  virtual void SetServerWallClockForTests(const SetServerWallClockForTestsRequestPB *req,
     SetServerWallClockForTestsResponsePB *resp, ::ant::rpc::RpcContext *context) = 0;
  virtual void GetStatus(const GetStatusRequestPB *req,
     GetStatusResponsePB *resp, ::ant::rpc::RpcContext *context) = 0;

};

} // namespace server
} // namespace ant

#endif