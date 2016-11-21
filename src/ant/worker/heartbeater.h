#ifndef ANT_SERVER_WORKER_NODE_HEARTBEATER_H_
#define ANT_SERVER_WORKER_NODE_HEARTBEATER_H_

#include <memory>
#include <string>
#include <vector>

#include "ant/base/macros.h"
#include "ant/util/status.h"

namespace ant {

namespace worker_node {

class WorkerServer;
struct WorkerServerOptions;

class Heartbeater {
 public:
  Heartbeater(const WorkerServerOptions& options, WorkerServer* server);
  Status Start();
  Status Stop();
  void TriggerASAP();
  //TODO(wqx):
  //
 private:
  class Thread;
  std::vector<std::unique_ptr<Thread>> threads_;
  DISALLOW_COPY_AND_ASSIGN(Heartbeater);
};

} // namespace worker_node
} // namespace ant
#endif // ANT_SERVER_WORKER_NODE_HEARTBEATER_H_
