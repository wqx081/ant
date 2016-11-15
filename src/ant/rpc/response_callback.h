#ifndef KUDU_RPC_RESPONSE_CALLBACK_H
#define KUDU_RPC_RESPONSE_CALLBACK_H

#include <functional>

namespace ant {
namespace rpc {

typedef std::function<void()> ResponseCallback;

}
}

#endif
