#include "ant/master/worker_manager.h"

#include <mutex>
#include <vector>

#include "ant/base/map-util.h"
#include "ant/base/strings/substitute.h"
#include "ant/master/master.pb.h"
#include "ant/master/worker_descriptor.h"

using std::shared_ptr;
using std::string;
using std::vector;
using strings::Substitute;

namespace ant {
namespace master {

WorkerManager::WorkerManager() {
}

WorkerManager::~WorkerManager() {
}

Status WorkerManager::LookupWorker(const NodeInstancePB& instance,
                           shared_ptr<WorkerDescriptor>* ts_desc) const {
  shared_lock<rw_spinlock> l(lock_);
  const shared_ptr<WorkerDescriptor>* found_ptr =
    FindOrNull(servers_by_id_, instance.permanent_uuid());
  if (!found_ptr) {
    return Status::NotFound("unknown tablet server ID", instance.ShortDebugString());
  }
  const shared_ptr<WorkerDescriptor>& found = *found_ptr;

  if (instance.instance_seqno() != found->latest_seqno()) {
    return Status::NotFound("mismatched instance sequence number", instance.ShortDebugString());
  }

  *ts_desc = found;
  return Status::OK();
}

bool WorkerManager::LookupWorkerByUUID(const string& uuid,
                               std::shared_ptr<WorkerDescriptor>* ts_desc) const {
  shared_lock<rw_spinlock> l(lock_);
  return FindCopy(servers_by_id_, uuid, ts_desc);
}

Status WorkerManager::RegisterWorker(const NodeInstancePB& instance,
                             const ServerRegistrationPB& registration,
                             std::shared_ptr<WorkerDescriptor>* desc) {
  std::lock_guard<rw_spinlock> l(lock_);
  const string& uuid = instance.permanent_uuid();

  if (!ContainsKey(servers_by_id_, uuid)) {
    shared_ptr<WorkerDescriptor> new_desc;
    RETURN_NOT_OK(WorkerDescriptor::RegisterNew(instance, registration, &new_desc));
    InsertOrDie(&servers_by_id_, uuid, new_desc);
    LOG(INFO) << Substitute("Registered new tserver with Master: $0",
                            new_desc->ToString());
    desc->swap(new_desc);
  } else {
    shared_ptr<WorkerDescriptor> found(FindOrDie(servers_by_id_, uuid));
    RETURN_NOT_OK(found->Register(instance, registration));
    LOG(INFO) << Substitute("Re-registered known tserver with Master: $0",
                            found->ToString());
    desc->swap(found);
  }

  return Status::OK();
}

void WorkerManager::GetAllDescriptors(vector<shared_ptr<WorkerDescriptor> > *descs) const {
  descs->clear();
  shared_lock<rw_spinlock> l(lock_);
  AppendValuesFromMap(servers_by_id_, descs);
}

void WorkerManager::GetAllLiveDescriptors(vector<shared_ptr<WorkerDescriptor> > *descs) const {
  descs->clear();

  shared_lock<rw_spinlock> l(lock_);
  descs->reserve(servers_by_id_.size());
  for (const WorkerDescriptorMap::value_type& entry : servers_by_id_) {
    const shared_ptr<WorkerDescriptor>& ts = entry.second;
    if (!ts->PresumedDead()) {
      descs->push_back(ts);
    }
  }
}

int WorkerManager::GetCount() const {
  shared_lock<rw_spinlock> l(lock_);
  return servers_by_id_.size();
}

} // namespace master
} // namespace ant

