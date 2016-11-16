#include "ant/rpc/proxy.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <inttypes.h>
#include <memory>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "ant/base/stringprintf.h"
#include "ant/base/strings/substitute.h"
#include "ant/rpc/outbound_call.h"
#include "ant/rpc/messenger.h"
#include "ant/rpc/remote_method.h"
#include "ant/rpc/response_callback.h"
#include "ant/rpc/rpc_header.pb.h"
#include "ant/util/net/sockaddr.h"
#include "ant/util/net/socket.h"
#include "ant/util/countdown_latch.h"
#include "ant/util/status.h"
#include "ant/util/user.h"

using google::protobuf::Message;
using std::string;
using std::shared_ptr;

namespace ant {
namespace rpc {

Proxy::Proxy(const std::shared_ptr<Messenger>& messenger,
             const Sockaddr& remote, string service_name)
    : service_name_(std::move(service_name)),
      messenger_(messenger),
      is_started_(false) {
  CHECK(messenger != nullptr);
  DCHECK(!service_name_.empty()) << "Proxy service name must not be blank";

  // By default, we set the real user to the currently logged-in user.
  // Effective user and password remain blank.
  string real_user;
  Status s = GetLoggedInUser(&real_user);
  if (!s.ok()) {
    LOG(WARNING) << "Proxy for " << service_name_ << ": Unable to get logged-in user name: "
        << s.ToString() << " before connecting to remote: " << remote.ToString();
  }

  conn_id_.set_remote(remote);
  conn_id_.mutable_user_credentials()->set_real_user(real_user);
}

Proxy::~Proxy() {
}

void Proxy::AsyncRequest(const string& method,
                         const google::protobuf::Message& req,
                         google::protobuf::Message* response,
                         RpcController* controller,
                         const ResponseCallback& callback) const {
  CHECK(controller->call_.get() == nullptr) << "Controller should be reset";
  base::subtle::NoBarrier_Store(&is_started_, true);
  RemoteMethod remote_method(service_name_, method);
  OutboundCall* call = new OutboundCall(conn_id_, remote_method, response, controller, callback);
  controller->call_.reset(call);
  call->SetRequestParam(req);

  // If this fails to queue, the callback will get called immediately
  // and the controller will be in an ERROR state.
  messenger_->QueueOutboundCall(controller->call_);
}


Status Proxy::SyncRequest(const string& method,
                          const google::protobuf::Message& req,
                          google::protobuf::Message* resp,
                          RpcController* controller) const {
  CountDownLatch latch(1);
  AsyncRequest(method, req, DCHECK_NOTNULL(resp), controller,
               boost::bind(&CountDownLatch::CountDown, boost::ref(latch)));

  latch.Wait();
  return controller->status();
}

void Proxy::set_user_credentials(const UserCredentials& user_credentials) {
  CHECK(base::subtle::NoBarrier_Load(&is_started_) == false)
    << "It is illegal to call set_user_credentials() after request processing has started";
  conn_id_.set_user_credentials(user_credentials);
}

std::string Proxy::ToString() const {
  return strings::Substitute("$0@$1", service_name_, conn_id_.ToString());
}

} // namespace rpc
} // namespace kudu
