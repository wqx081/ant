#ifndef ANT_WORKER_SERVER_WORKER_SERVER_H_
#define ANT_WORKER_SERVER_WORKER_SERVER_H_

#include <memory>
#include <string>
#include <vector>

#include "ant/base/atomicops.h"
#include "ant/base/gscoped_ptr.h"
#include "ant/base/macros.h"

#include "ant/server/server_base.h"
#include "ant/server/web_server.h"
#include "ant/worker_server/worker_server_options.h"
#include "ant/util/net/net_util.h"
#include "ant/util/net/sockaddr.h"
#include "ant/util/status.h"

namespace ant {
namespace worker_server {

class Heartbeater;

class WorkerServer : public server::ServerBase {
 public:
  static const uint16_t kDefaultPort = 7050;
  static const uint16_t kDefaultWebPort = 8050;

  explicit WorkerServer(const WorkerServerOptions& opts);
  ~WorkerServer();


  Status Init();
  Status WaitInited();

  Status Start();
  void Shutdown();

  std::string ToString() const;

  Heartbeater* heartbeater() { return heartbeater_.get(); }

  void set_fail_heartbeats_for_tests(bool fail_heartbeats_for_tests) {
    base::subtle::NoBarrier_Store(&fail_heartbeats_for_tests_, 1);
  } 
  
  bool fail_heartbeats_for_tests() const {
    return base::subtle::NoBarrier_Load(&fail_heartbeats_for_tests_);
  } 

 private:
  Status ValidateMasterAddressResolution() const;

  bool initted_;
  
  base::subtle::Atomic32 fail_heartbeats_for_tests_;

  const WorkerServerOptions opts_;

  gscoped_ptr<Heartbeater> heartbeater_;

  DISALLOW_COPY_AND_ASSIGN(WorkerServer);
};

} // namespace worker_server
} // namespace ant
#endif // ANT_WORKER_SERVER_WORKER_SERVER_H_
