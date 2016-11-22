#include <glog/logging.h>
#include <gflags/gflags.h>

#include <iostream>

#include "ant/base/strings/substitute.h"

#include "ant/worker_server/worker_server.h"
#include "ant/common/version_info.h"

using ant::worker_server::WorkerServer;

DECLARE_string(rpc_bind_addresses);
DECLARE_int32(rpc_num_service_threads);
DECLARE_int32(webserver_port);

namespace ant {
namespace worker_server {

static int WorkerServerMain(int argc, char** argv) {

  FLAGS_rpc_bind_addresses = strings::Substitute("0.0.0.0:$0",
                                                 WorkerServer::kDefaultPort);
  FLAGS_rpc_num_service_threads = 20;
  FLAGS_webserver_port = WorkerServer::kDefaultWebPort;


  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 1) {
    std::cerr << "usage: " << argv[0] << std::endl;
    return 1;
  }

  WorkerServerOptions opts;
  WorkerServer server(opts);
  LOG(INFO) << "Initializing tablet server...";
  CHECK_OK(server.Init());

  LOG(INFO) << "Starting tablet server...";
  CHECK_OK(server.Start());

  LOG(INFO) << "Worker server successfully started.";
  while (true) {
    SleepFor(MonoDelta::FromSeconds(60));
  }

  return 0;
}

} // namespace worker_server
} // namespace kudu

int main(int argc, char** argv) {
  return ant::worker_server::WorkerServerMain(argc, argv);
}
