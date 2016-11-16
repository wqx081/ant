#include "ant/rpc/blocking_ops.h"

#include <stdint.h>
#include <string.h>

#include <glog/logging.h>
#include <google/protobuf/message_lite.h>

#include "ant/base/endian.h"
#include "ant/base/strings/substitute.h"
#include "ant/rpc/constants.h"
#include "ant/rpc/serialization.h"
#include "ant/rpc/transfer.h"
#include "ant/util/faststring.h"
#include "ant/util/monotime.h"
#include "ant/util/net/sockaddr.h"
#include "ant/util/net/socket.h"
#include "ant/util/status.h"

namespace ant {
namespace rpc {

using google::protobuf::MessageLite;

const char kHTTPHeader[] = "HTTP";

Status EnsureBlockingMode(const Socket* const sock) {
  bool is_nonblocking;
  RETURN_NOT_OK(sock->IsNonBlocking(&is_nonblocking));
  if (is_nonblocking) {
    return Status::IllegalState("Underlying socket is not set to blocking mode!");
  }
  return Status::OK();
}

Status SendFramedMessageBlocking(Socket* sock, const MessageLite& header, const MessageLite& msg,
    const MonoTime& deadline) {
  DCHECK(sock != nullptr);
  DCHECK(header.IsInitialized()) << "header protobuf must be initialized";
  DCHECK(msg.IsInitialized()) << "msg protobuf must be initialized";

  RETURN_NOT_OK(EnsureBlockingMode(sock));

  // Ensure we are in blocking mode.
  // These blocking calls are typically not in the fast path, so doing this for all build types.
  bool is_non_blocking = false;
  RETURN_NOT_OK(sock->IsNonBlocking(&is_non_blocking));
  DCHECK(!is_non_blocking) << "Socket must be in blocking mode to use SendFramedMessage";

  // Serialize message
  faststring param_buf;
  serialization::SerializeMessage(msg, &param_buf);

  // Serialize header and initial length
  faststring header_buf;
  serialization::SerializeHeader(header, param_buf.size(), &header_buf);

  // Write header & param to stream
  size_t nsent;
  RETURN_NOT_OK(sock->BlockingWrite(header_buf.data(), header_buf.size(), &nsent, deadline));
  RETURN_NOT_OK(sock->BlockingWrite(param_buf.data(), param_buf.size(), &nsent, deadline));

  return Status::OK();
}

Status ReceiveFramedMessageBlocking(Socket* sock, faststring* recv_buf,
    MessageLite* header, Slice* param_buf, const MonoTime& deadline) {
  DCHECK(sock != nullptr);
  DCHECK(recv_buf != nullptr);
  DCHECK(header != nullptr);
  DCHECK(param_buf != nullptr);

  RETURN_NOT_OK(EnsureBlockingMode(sock));

  // Read the message prefix, which specifies the length of the payload.
  recv_buf->clear();
  recv_buf->resize(kMsgLengthPrefixLength);
  size_t recvd = 0;
  RETURN_NOT_OK(sock->BlockingRecv(recv_buf->data(), kMsgLengthPrefixLength, &recvd, deadline));
  uint32_t payload_len = NetworkByteOrder::Load32(recv_buf->data());

  // Verify that the payload size isn't out of bounds.
  // This can happen because of network corruption, or a naughty client.
  if (PREDICT_FALSE(payload_len > FLAGS_rpc_max_message_size)) {
    // A common user mistake is to try to speak the Kudu RPC protocol to an
    // HTTP endpoint, or vice versa.
    if (memcmp(recv_buf->data(), kHTTPHeader, strlen(kHTTPHeader)) == 0) {
      return Status::IOError(
          "received invalid RPC message which appears to be an HTTP response. "
          "Verify that you have specified a valid RPC port and not an HTTP port.");
    }

    return Status::IOError(
        strings::Substitute(
            "received invalid message of size $0 which exceeds"
            " the rpc_max_message_size of $1 bytes",
            payload_len, FLAGS_rpc_max_message_size));
  }

  // Read the message payload.
  recvd = 0;
  recv_buf->resize(payload_len + kMsgLengthPrefixLength);
  RETURN_NOT_OK(sock->BlockingRecv(recv_buf->data() + kMsgLengthPrefixLength,
                payload_len, &recvd, deadline));
  RETURN_NOT_OK(serialization::ParseMessage(Slice(*recv_buf), header, param_buf));
  return Status::OK();
}

} // namespace rpc
} // namespace kudu
