#include "ant/rpc/sasl_helper.h"

#include <set>
#include <string>

#include <glog/logging.h>
#include <google/protobuf/message_lite.h>

#include "ant/base/endian.h"
#include "ant/base/gscoped_ptr.h"
#include "ant/base/macros.h"
#include "ant/base/map-util.h"
#include "ant/base/port.h"
#include "ant/base/strings/join.h"
#include "ant/rpc/blocking_ops.h"
#include "ant/rpc/constants.h"
#include "ant/rpc/rpc_header.pb.h"
#include "ant/rpc/sasl_common.h"
#include "ant/rpc/serialization.h"
#include "ant/util/faststring.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"

namespace ant {
namespace rpc {

using google::protobuf::MessageLite;

SaslHelper::SaslHelper(PeerType peer_type)
  : peer_type_(peer_type),
    conn_header_exchanged_(false),
    anonymous_enabled_(false),
    plain_enabled_(false),
    gssapi_enabled_(false) {
  tag_ = (peer_type_ == SERVER) ? "Sasl Server" : "Sasl Client";
}

SaslHelper::~SaslHelper() {
}

void SaslHelper::set_local_addr(const Sockaddr& addr) {
  local_addr_ = SaslIpPortString(addr);
}
const char* SaslHelper::local_addr_string() const {
  return local_addr_.empty() ? nullptr : local_addr_.c_str();
}

void SaslHelper::set_remote_addr(const Sockaddr& addr) {
  remote_addr_ = SaslIpPortString(addr);
}
const char* SaslHelper::remote_addr_string() const {
  return remote_addr_.empty() ? nullptr : remote_addr_.c_str();
}

void SaslHelper::set_server_fqdn(const string& domain_name) {
  server_fqdn_ = domain_name;
}
const char* SaslHelper::server_fqdn() const {
  return server_fqdn_.empty() ? nullptr : server_fqdn_.c_str();
}

const std::set<std::string>& SaslHelper::GlobalMechs() const {
  if (!global_mechs_) {
    global_mechs_.reset(new set<string>(SaslListAvailableMechs()));
  }
  return *global_mechs_;
}

void SaslHelper::AddToLocalMechList(const string& mech) {
  mechs_.insert(mech);
}

const std::set<std::string>& SaslHelper::LocalMechs() const {
  return mechs_;
}

const char* SaslHelper::LocalMechListString() const {
  JoinStrings(mechs_, " ", &mech_list_);
  return mech_list_.c_str();
}


int SaslHelper::GetOptionCb(const char* plugin_name, const char* option,
                            const char** result, unsigned* len) {
  DVLOG(4) << tag_ << ": GetOption Callback called. ";
  DVLOG(4) << tag_ << ": GetOption Plugin name: "
                   << (plugin_name == nullptr ? "NULL" : plugin_name);
  DVLOG(4) << tag_ << ": GetOption Option name: " << option;

  if (PREDICT_FALSE(result == nullptr)) {
    LOG(DFATAL) << tag_ << ": SASL Library passed NULL result out-param to GetOption callback!";
    return SASL_BADPARAM;
  }

  if (plugin_name == nullptr) {
    // SASL library option, not a plugin option
    if (strcmp(option, "mech_list") == 0) {
      *result = LocalMechListString();
      if (len != nullptr) *len = strlen(*result);
      VLOG(4) << tag_ << ": Enabled mech list: " << *result;
      return SASL_OK;
    }
    VLOG(4) << tag_ << ": GetOptionCb: Unknown library option: " << option;
  } else {
    VLOG(4) << tag_ << ": GetOptionCb: Unknown plugin: " << plugin_name;
  }
  return SASL_FAIL;
}

Status SaslHelper::EnableAnonymous() {
  RETURN_NOT_OK(EnableMechanism(kSaslMechAnonymous));
  anonymous_enabled_ = true;
  return Status::OK();
}

bool SaslHelper::IsAnonymousEnabled() const {
  return anonymous_enabled_;
}

Status SaslHelper::EnablePlain() {
  RETURN_NOT_OK(EnableMechanism(kSaslMechPlain));
  plain_enabled_ = true;
  return Status::OK();
}

Status SaslHelper::EnableGSSAPI() {
  RETURN_NOT_OK(EnableMechanism(kSaslMechGSSAPI));
  gssapi_enabled_ = true;
  return Status::OK();
}

Status SaslHelper::EnableMechanism(const string& mech) {
  if (PREDICT_FALSE(!ContainsKey(GlobalMechs(), mech))) {
    return Status::InvalidArgument("unable to find SASL plugin", mech);
  }
  AddToLocalMechList(mech);
  return Status::OK();
}

bool SaslHelper::IsPlainEnabled() const {
  return plain_enabled_;
}

Status SaslHelper::SanityCheckSaslCallId(int32_t call_id) const {
  if (call_id != kSaslCallId) {
    Status s = Status::IllegalState(StringPrintf("Non-SASL request during negotiation. "
          "Expected callId: %d, received callId: %d", kSaslCallId, call_id));
    LOG(DFATAL) << tag_ << ": " << s.ToString();
    return s;
  }
  return Status::OK();
}

Status SaslHelper::ParseSaslMessage(const Slice& param_buf, SaslMessagePB* msg) {
  if (!msg->ParseFromArray(param_buf.data(), param_buf.size())) {
    return Status::IOError(tag_ + ": Invalid SASL message, missing fields",
        msg->InitializationErrorString());
  }
  return Status::OK();
}

Status SaslHelper::SendSaslMessage(Socket* sock, const MessageLite& header, const MessageLite& msg,
      const MonoTime& deadline) {
  DCHECK(sock != nullptr);
  DCHECK(header.IsInitialized()) << tag_ << ": Header must be initialized";
  DCHECK(msg.IsInitialized()) << tag_ << ": Message must be initialized";

  // Write connection header, if needed
  if (PREDICT_FALSE(peer_type_ == CLIENT && !conn_header_exchanged_)) {
    const uint8_t buflen = kMagicNumberLength + kHeaderFlagsLength;
    uint8_t buf[buflen];
    serialization::SerializeConnHeader(buf);
    size_t nsent;
    RETURN_NOT_OK(sock->BlockingWrite(buf, buflen, &nsent, deadline));
    conn_header_exchanged_ = true;
  }

  RETURN_NOT_OK(SendFramedMessageBlocking(sock, header, msg, deadline));
  return Status::OK();
}

} // namespace rpc
} // namespace ant
