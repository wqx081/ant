#ifndef ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
#define ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
#include <vector>
#include "ant/server/server_base_options.h"
#include "ant/util/net/net_util.h"

namespace ant {
namespace worker_server {

struct WorkerServerOptions : public ant::server::ServerBaseOptions {
  WorkerServerOptions();

  std::vector<HostPort> master_addresses;
};

} // namespace worker_server
} // namespace ant
#endif // ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
