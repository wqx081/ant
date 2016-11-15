#include "ant/rpc/constants.h"

using std::set;

namespace ant {
namespace rpc {

const char* const kMagicNumber = "hrpc";
const char* const kSaslAppName = "kudu";
const char* const kSaslProtoName = "kudu";
set<RpcFeatureFlag> kSupportedServerRpcFeatureFlags = { APPLICATION_FEATURE_FLAGS };
set<RpcFeatureFlag> kSupportedClientRpcFeatureFlags = { APPLICATION_FEATURE_FLAGS };

} // namespace rpc
} // namespace ant
