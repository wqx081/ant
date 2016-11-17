// THIS FILE IS AUTOGENERATED FROM ant/tests/rpc/rtest.proto

#include "ant/tests/rpc/rtest.proxy.pb.h"

#include "ant/rpc/outbound_call.h"
#include "ant/util/net/sockaddr.h"

namespace ant {
namespace rpc_test {

CalculatorServiceProxy::CalculatorServiceProxy(
   const std::shared_ptr< ::ant::rpc::Messenger> &messenger,
   const ::ant::Sockaddr &remote)
  : Proxy(messenger, remote, "ant.rpc_test.CalculatorService") {
}

CalculatorServiceProxy::~CalculatorServiceProxy() {
}


::ant::Status CalculatorServiceProxy::Add(const AddRequestPB &req, AddResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("Add", req, resp, controller);
}

void CalculatorServiceProxy::AddAsync(const AddRequestPB &req,
                     AddResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("Add", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::Sleep(const SleepRequestPB &req, SleepResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("Sleep", req, resp, controller);
}

void CalculatorServiceProxy::SleepAsync(const SleepRequestPB &req,
                     SleepResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("Sleep", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::Echo(const EchoRequestPB &req, EchoResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("Echo", req, resp, controller);
}

void CalculatorServiceProxy::EchoAsync(const EchoRequestPB &req,
                     EchoResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("Echo", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::WhoAmI(const WhoAmIRequestPB &req, WhoAmIResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("WhoAmI", req, resp, controller);
}

void CalculatorServiceProxy::WhoAmIAsync(const WhoAmIRequestPB &req,
                     WhoAmIResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("WhoAmI", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::TestArgumentsInDiffPackage(const ant::rpc_test_diff_package::ReqDiffPackagePB &req, ant::rpc_test_diff_package::RespDiffPackagePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("TestArgumentsInDiffPackage", req, resp, controller);
}

void CalculatorServiceProxy::TestArgumentsInDiffPackageAsync(const ant::rpc_test_diff_package::ReqDiffPackagePB &req,
                     ant::rpc_test_diff_package::RespDiffPackagePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("TestArgumentsInDiffPackage", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::Panic(const PanicRequestPB &req, PanicResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("Panic", req, resp, controller);
}

void CalculatorServiceProxy::PanicAsync(const PanicRequestPB &req,
                     PanicResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("Panic", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::AddExactlyOnce(const ExactlyOnceRequestPB &req, ExactlyOnceResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("AddExactlyOnce", req, resp, controller);
}

void CalculatorServiceProxy::AddExactlyOnceAsync(const ExactlyOnceRequestPB &req,
                     ExactlyOnceResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("AddExactlyOnce", req, resp, controller, callback);
}

::ant::Status CalculatorServiceProxy::TestInvalidResponse(const TestInvalidResponseRequestPB &req, TestInvalidResponseResponsePB *resp,
                                     ::ant::rpc::RpcController *controller) {
  return SyncRequest("TestInvalidResponse", req, resp, controller);
}

void CalculatorServiceProxy::TestInvalidResponseAsync(const TestInvalidResponseRequestPB &req,
                     TestInvalidResponseResponsePB *resp, ::ant::rpc::RpcController *controller,
                     const ::ant::rpc::ResponseCallback &callback) {
  AsyncRequest("TestInvalidResponse", req, resp, controller, callback);
}

} // namespace rpc_test
} // namespace ant
