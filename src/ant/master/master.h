#ifndef ANT_MASTER_MASTER_H_
#define ANT_MASTER_MASTER_H_

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "ant/common/wire_protocol.h"
#include "ant/base/gscoped_ptr.h"
#include "ant/base/macros.h"
#include "ant/master/master_options.h"
#include "ant/master/master.pb.h"
#include "ant/server/server_base.h"
#include "ant/util/metrics.h"
#include "ant/util/promise.h"
#include "ant/util/status.h"


namespace ant {

class RpcServer;
struct RpcServerOptions;
class ThreadPool;

namespace rpc {
class Messenger;
class ServicePool;
} // namespace rpc

namespace master {

class WorkerManager;

class Master : public server::ServerBase {
 public:
  static const uint16_t kDefaultPort = 7051;
  static const uint16_t kDefaultWebPort = 8051;

  explicit Master(const MasterOptions& opts);
  ~Master();

  Status Init();
  Status Start();

  Status StartAsync();

  void Shutdown();

  std::string ToString() const;

  WorkerManager* worker_manager() { return worker_manager_.get(); }
  const MasterOptions& opts() { return opts_; }

  Status GetMasterRegistration(ServerRegistrationPB* registration) const;
  bool IsShutdown() const {
    return state_ == kStopped;
  }

 private:

  Status InitMasterRegistration(); 

  enum MasterState {
    kStopped,
    kInitialized,
    kRunning,
  };
  MasterState state_;

  gscoped_ptr<WorkerManager> worker_manager_;

  gscoped_ptr<ThreadPool> init_pool_;
  Promise<Status> init_status_;
  
  MasterOptions opts_;

  ServerRegistrationPB registration_;
  // True once registration_ has been initialized.
  std::atomic<bool> registration_initialized_;

  DISALLOW_COPY_AND_ASSIGN(Master);
};

} // namespace master
} // namespace ant
#endif // ANT_MASTER_MASTER_H_
