#include <glog/logging.h>
#include <iostream>

#include "ant/base/strings/substitute.h"
#include "ant/master/master.h"
#include "ant/common/version_info.h"
#include "ant/util/logging.h"

using ant::master::Master;

DECLARE_string(rpc_bind_addresses);
DECLARE_int32(webserver_port);

namespace ant {
namespace master {

static int MasterMain(int argc, char** argv) {

  FLAGS_rpc_bind_addresses = strings::Substitute("0.0.0.0:$0",
                                                 Master::kDefaultPort);
  FLAGS_webserver_port = Master::kDefaultWebPort;

  google::ParseCommandLineFlags(&argc, &argv, true);
  ant::InitGoogleLoggingSafe(argv[0]);

  MasterOptions opts;
  Master server(opts);
  LOG(INFO) << "Initializing master server...";
  CHECK_OK(server.Init());

  LOG(INFO) << "Starting Master server...";
  CHECK_OK(server.Start());

  LOG(INFO) << "Master server successfully started.";
  while (true) {
    SleepFor(MonoDelta::FromSeconds(60));
  }

  return 0;
}

} // namespace master
} // namespace ant

int main(int argc, char** argv) {
  return ant::master::MasterMain(argc, argv);
}
