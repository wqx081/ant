#ifndef KUDU_RPC_SASL_CLIENT_H
#define KUDU_RPC_SASL_CLIENT_H

#include <set>
#include <string>
#include <vector>

#include <sasl/sasl.h>

#include "ant/base/gscoped_ptr.h"
#include "ant/rpc/rpc_header.pb.h"
#include "ant/rpc/sasl_common.h"
#include "ant/rpc/sasl_helper.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"
#include "ant/util/net/socket.h"

namespace ant {
namespace rpc {

using std::string;

class ResponseHeader;
class SaslMessagePB;
class SaslMessagePB_SaslAuth;

// Class for doing SASL negotiation with a SaslServer over a bidirectional socket.
// Operations on this class are NOT thread-safe.
class SaslClient {
 public:
  // Does not take ownership of the socket indicated by the fd.
  SaslClient(string app_name, Socket* socket);

  // Enable ANONYMOUS authentication.
  // Must be called after Init().
  Status EnableAnonymous();

  // Enable PLAIN authentication.
  // Must be called after Init().
  Status EnablePlain(const string& user, const string& pass);

  // Enable GSSAPI authentication.
  // Call after Init().
  Status EnableGSSAPI();

  // Returns mechanism negotiated by this connection.
  // Must be called after Negotiate().
  SaslMechanism::Type negotiated_mechanism() const;

  // Returns the set of RPC system features supported by the remote server.
  // Must be called after Negotiate().
  const std::set<RpcFeatureFlag>& server_features() const {
    return server_features_;
  }

  // Specify IP:port of local side of connection.
  // Must be called before Init(). Required for some mechanisms.
  void set_local_addr(const Sockaddr& addr);

  // Specify IP:port of remote side of connection.
  // Must be called before Init(). Required for some mechanisms.
  void set_remote_addr(const Sockaddr& addr);

  // Specify the fully-qualified domain name of the remote server.
  // Must be called before Init(). Required for some mechanisms.
  void set_server_fqdn(const string& domain_name);

  // Set deadline for connection negotiation.
  void set_deadline(const MonoTime& deadline);

  // Get deadline for connection negotiation.
  const MonoTime& deadline() const { return deadline_; }

  // Initialize a new SASL client. Must be called before Negotiate().
  // Returns OK on success, otherwise RuntimeError.
  Status Init(const string& service_type);

  // Begin negotiation with the SASL server on the other side of the fd socket
  // that this client was constructed with.
  // Returns OK on success.
  // Otherwise, it may return NotAuthorized, NotSupported, or another non-OK status.
  Status Negotiate();

  // SASL callback for plugin options, supported mechanisms, etc.
  // Returns SASL_FAIL if the option is not handled, which does not fail the handshake.
  int GetOptionCb(const char* plugin_name, const char* option,
                  const char** result, unsigned* len);

  // SASL callback for SASL_CB_USER, SASL_CB_AUTHNAME, SASL_CB_LANGUAGE
  int SimpleCb(int id, const char** result, unsigned* len);

  // SASL callback for SASL_CB_PASS
  int SecretCb(sasl_conn_t* conn, int id, sasl_secret_t** psecret);

 private:
  // Encode and send the specified SASL message to the server.
  Status SendSaslMessage(const SaslMessagePB& msg);

  // Validate that header does not indicate an error, parse param_buf into response.
  Status ParseSaslMsgResponse(const ResponseHeader& header, const Slice& param_buf,
      SaslMessagePB* response);

  // Send an NEGOTIATE message to the server.
  Status SendNegotiateMessage();

  // Send an INITIATE message to the server.
  Status SendInitiateMessage(const SaslMessagePB_SaslAuth& auth,
                             const char* init_msg, unsigned init_msg_len);

  // Send a RESPONSE message to the server.
  Status SendResponseMessage(const char* resp_msg, unsigned resp_msg_len);

  // Perform a client-side step of the SASL negotiation.
  // Input is what came from the server. Output is what we will send back to the server.
  // Returns:
  //   Status::OK if sasl_client_step returns SASL_OK.
  //   Status::Incomplete if sasl_client_step returns SASL_CONTINUE
  // otherwise returns an appropriate error status.
  Status DoSaslStep(const string& in, const char** out, unsigned* out_len);

  // Handle case when server sends NEGOTIATE response.
  Status HandleNegotiateResponse(const SaslMessagePB& response);

  // Handle case when server sends CHALLENGE response.
  Status HandleChallengeResponse(const SaslMessagePB& response);

  // Handle case when server sends SUCCESS response.
  Status HandleSuccessResponse(const SaslMessagePB& response);

  // Parse error status message from raw bytes of an ErrorStatusPB.
  Status ParseError(const Slice& err_data);

  string app_name_;
  Socket* sock_;
  std::vector<sasl_callback_t> callbacks_;
  // The SASL connection object. This is initialized in Init() and
  // freed after Negotiate() completes (regardless whether it was successful).
  gscoped_ptr<sasl_conn_t, SaslDeleter> sasl_conn_;
  SaslHelper helper_;

  string plain_auth_user_;
  string plain_pass_;
  gscoped_ptr<sasl_secret_t, base::FreeDeleter> psecret_;

  // The set of features supported by the server.
  std::set<RpcFeatureFlag> server_features_;

  SaslNegotiationState::Type client_state_;

  // The mechanism we negotiated with the server.
  SaslMechanism::Type negotiated_mech_;

  // Intra-negotiation state.
  bool nego_ok_;  // During negotiation: did we get a SASL_OK response from the SASL library?
  bool nego_response_expected_;  // During negotiation: Are we waiting for a server response?

  // Negotiation timeout deadline.
  MonoTime deadline_;

  DISALLOW_COPY_AND_ASSIGN(SaslClient);
};

} // namespace rpc
} // namespace kudu

#endif  // KUDU_RPC_SASL_CLIENT_H
