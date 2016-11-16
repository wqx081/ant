#include "ant/rpc/negotiation.h"

#include <sys/time.h>
#include <poll.h>

#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ant/base/stringprintf.h"
#include "ant/base/strings/substitute.h"
#include "ant/rpc/blocking_ops.h"
#include "ant/rpc/connection.h"
#include "ant/rpc/reactor.h"
#include "ant/rpc/rpc_header.pb.h"
#include "ant/rpc/sasl_client.h"
#include "ant/rpc/sasl_common.h"
#include "ant/rpc/sasl_server.h"
#include "ant/util/errno.h"
//#include "ant/util/flag_tags.h"
#include "ant/util/status.h"
#include "ant/util/trace.h"

DEFINE_bool(rpc_trace_negotiation, false,
            "If enabled, dump traces of all RPC negotiations to the log");
//// TAG_FLAG(rpc_trace_negotiation, runtime);
//// TAG_FLAG(rpc_trace_negotiation, advanced);
//// TAG_FLAG(rpc_trace_negotiation, experimental);

DEFINE_int32(rpc_negotiation_inject_delay_ms, 0,
             "If enabled, injects the given number of milliseconds delay into "
             "the RPC negotiation process on the server side.");
//// TAG_FLAG(rpc_negotiation_inject_delay_ms, unsafe);

namespace ant {
namespace rpc {

using std::shared_ptr;
using strings::Substitute;

// Client: Send ConnectionContextPB message based on information stored in the Connection object.
static Status SendConnectionContext(Connection* conn, const MonoTime& deadline) {
  TRACE("Sending connection context");
  RequestHeader header;
  header.set_call_id(kConnectionContextCallId);

  ConnectionContextPB conn_context;
  // This field is deprecated but used by servers <Kudu 1.1. Newer server versions ignore
  // this and use the SASL-provided username instead.
  conn_context.mutable_deprecated_user_info()->set_real_user(
      conn->user_credentials().real_user());
  return SendFramedMessageBlocking(conn->socket(), header, conn_context, deadline);
}

// Server: Receive ConnectionContextPB message and update the corresponding fields in the
// associated Connection object. Perform validation against SASL-negotiated information
// as needed.
static Status RecvConnectionContext(Connection* conn, const MonoTime& deadline) {
  TRACE("Waiting for connection context");
  faststring recv_buf(1024); // Should be plenty for a ConnectionContextPB message.
  RequestHeader header;
  Slice param_buf;
  RETURN_NOT_OK(ReceiveFramedMessageBlocking(conn->socket(), &recv_buf,
                                             &header, &param_buf, deadline));
  DCHECK(header.IsInitialized());

  if (header.call_id() != kConnectionContextCallId) {
    return Status::IllegalState("Expected ConnectionContext callid, received",
        Substitute("$0", header.call_id()));
  }

  ConnectionContextPB conn_context;
  if (!conn_context.ParseFromArray(param_buf.data(), param_buf.size())) {
    return Status::InvalidArgument("Invalid ConnectionContextPB message, missing fields",
                                   conn_context.InitializationErrorString());
  }

  if (conn->sasl_server().authenticated_user().empty()) {
    return Status::NotAuthorized("No user was authenticated");
  }

  conn->mutable_user_credentials()->set_real_user(conn->sasl_server().authenticated_user());

  return Status::OK();
}

// Wait for the client connection to be established and become ready for writing.
static Status WaitForClientConnect(Connection* conn, const MonoTime& deadline) {
  TRACE("Waiting for socket to connect");
  int fd = conn->socket()->GetFd();
  struct pollfd poll_fd;
  poll_fd.fd = fd;
  poll_fd.events = POLLOUT;
  poll_fd.revents = 0;

  MonoTime now;
  MonoDelta remaining;
  while (true) {
    now = MonoTime::Now();
    remaining = deadline - now;
    DVLOG(4) << "Client waiting to connect for negotiation, time remaining until timeout deadline: "
             << remaining.ToString();
    if (PREDICT_FALSE(remaining.ToNanoseconds() <= 0)) {
      return Status::TimedOut("Timeout exceeded waiting to connect");
    }
#if defined(__linux__)
    struct timespec ts;
    remaining.ToTimeSpec(&ts);
    int ready = ppoll(&poll_fd, 1, &ts, NULL);
#else
    int ready = poll(&poll_fd, 1, remaining.ToMilliseconds());
#endif
    if (ready == -1) {
      int err = errno;
      if (err == EINTR) {
        // We were interrupted by a signal, let's go again.
        continue;
      } else {
        return Status::NetworkError("Error from ppoll() while waiting to connect",
            ErrnoToString(err), err);
      }
    } else if (ready == 0) {
      // Timeout exceeded. Loop back to the top to our impending doom.
      continue;
    } else {
      // Success.
      break;
    }
  }

  // Connect finished, but this doesn't mean that we connected successfully.
  // Check the socket for an error.
  int so_error = 0;
  socklen_t socklen = sizeof(so_error);
  int rc = getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &socklen);
  if (rc != 0) {
    return Status::NetworkError("Unable to check connected socket for errors",
                                ErrnoToString(errno),
                                errno);
  }
  if (so_error != 0) {
    return Status::NetworkError("connect", ErrnoToString(so_error), so_error);
  }

  return Status::OK();
}

// Disable / reset socket timeouts.
static Status DisableSocketTimeouts(Connection* conn) {
  RETURN_NOT_OK(conn->socket()->SetSendTimeout(MonoDelta::FromNanoseconds(0L)));
  RETURN_NOT_OK(conn->socket()->SetRecvTimeout(MonoDelta::FromNanoseconds(0L)));
  return Status::OK();
}

// Perform client negotiation. We don't LOG() anything, we leave that to our caller.
static Status DoClientNegotiation(Connection* conn,
                                  const MonoTime& deadline) {
  // The SASL initialization on the client side can be relatively heavy-weight
  // (it may result in DNS queries in the case of GSSAPI).
  // So, we do it while the connect() is still in progress to reduce latency.
  //
  // TODO(todd): we should consider doing this even before connecting, since as soon
  // as we connect, we are tying up a negotiation thread on the server side.

  RETURN_NOT_OK(conn->InitSaslClient());

  RETURN_NOT_OK(WaitForClientConnect(conn, deadline));
  RETURN_NOT_OK(conn->SetNonBlocking(false));
  RETURN_NOT_OK(conn->InitSSLIfNecessary());
  conn->sasl_client().set_deadline(deadline);
  RETURN_NOT_OK(conn->sasl_client().Negotiate());
  RETURN_NOT_OK(SendConnectionContext(conn, deadline));
  RETURN_NOT_OK(DisableSocketTimeouts(conn));

  return Status::OK();
}

// Perform server negotiation. We don't LOG() anything, we leave that to our caller.
static Status DoServerNegotiation(Connection* conn,
                                  const MonoTime& deadline) {
  if (FLAGS_rpc_negotiation_inject_delay_ms > 0) {
    LOG(WARNING) << "Injecting " << FLAGS_rpc_negotiation_inject_delay_ms
                 << "ms delay in negotiation";
    SleepFor(MonoDelta::FromMilliseconds(FLAGS_rpc_negotiation_inject_delay_ms));
  }
  RETURN_NOT_OK(conn->SetNonBlocking(false));
  RETURN_NOT_OK(conn->InitSSLIfNecessary());
  RETURN_NOT_OK(conn->InitSaslServer());
  conn->sasl_server().set_deadline(deadline);
  RETURN_NOT_OK(conn->sasl_server().Negotiate());
  RETURN_NOT_OK(RecvConnectionContext(conn, deadline));
  RETURN_NOT_OK(DisableSocketTimeouts(conn));

  return Status::OK();
}

// Perform negotiation for a connection (either server or client)
void Negotiation::RunNegotiation(const scoped_refptr<Connection>& conn,
                                 const MonoTime& deadline) {
  Status s;
  if (conn->direction() == Connection::SERVER) {
    s = DoServerNegotiation(conn.get(), deadline);
  } else {
    s = DoClientNegotiation(conn.get(), deadline);
  }

  if (PREDICT_FALSE(!s.ok())) {
    string msg = Substitute("$0 connection negotiation failed: $1",
                            conn->direction() == Connection::SERVER ? "Server" : "Client",
                            conn->ToString());
    s = s.CloneAndPrepend(msg);
  }
  TRACE("Negotiation complete: $0", s.ToString());

  bool is_bad = !s.ok() && !(
      (s.IsNetworkError() && s.posix_code() == ECONNREFUSED) ||
      s.IsNotAuthorized());

  if (is_bad || FLAGS_rpc_trace_negotiation) {
    string msg = Trace::CurrentTrace()->DumpToString();
    if (is_bad) {
      LOG(WARNING) << "Failed RPC negotiation. Trace:\n" << msg;
    } else {
      LOG(INFO) << "RPC negotiation tracing enabled. Trace:\n" << msg;
    }
  }

  if (conn->direction() == Connection::SERVER && s.IsNotAuthorized()) {
    LOG(WARNING) << "Unauthorized connection attempt: " << s.message().ToString();
  }
  conn->CompleteNegotiation(s);
}


} // namespace rpc
} // namespace kudu
