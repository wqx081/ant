#ifndef ANT_MASTER_WORKER_MANAGER_H_
#define ANT_MASTER_WORKER_MANAGER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ant/base/macros.h"
#include "ant/util/locks.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"

namespace ant {

class NodeInstancePB;
class ServerRegistrationPB;

namespace master {

class WorkerDescriptor;

typedef std::vector<std::shared_ptr<WorkerDescriptor>> WorkerDescriptorVector;

class WorkerManager {
 public:
  WorkerManager();
  virtual ~WorkerManager();

  Status LookupWorker(const NodeInstancePB& instance,
		      std::shared_ptr<WorkerDescriptor>* desc) const;
  bool LookupWorkerByUUID(const std::string& uuid,
		      std::shared_ptr<WorkerDescriptor>* desc) const;
		          
  Status RegisterWorker(const NodeInstancePB& instance,
		        const ServerRegistrationPB& registration,
			std::shared_ptr<WorkerDescriptor>* desc);

  void GetAllDescriptors(std::vector<std::shared_ptr<WorkerDescriptor>>*
		         descs) const;
  void GetAllLiveDescriptors(std::vector<std::shared_ptr<WorkerDescriptor>>*
		         descs) const;

  int GetCount() const;

 private:
  mutable rw_spinlock lock_;

  typedef std::unordered_map<std::string,
	                     std::shared_ptr<WorkerDescriptor>> 
				     WorkerDescriptorMap;
  WorkerDescriptorMap servers_by_id_;

  DISALLOW_COPY_AND_ASSIGN(WorkerManager);
};

} // namespace masterk
} // namespace ant
#endif // ANT_MASTER_WORKER_MANAGER_H_
