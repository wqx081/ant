#ifndef KUDU_UTIL_SPINLOCK_PROFILING_H
#define KUDU_UTIL_SPINLOCK_PROFILING_H

#include <iosfwd>

#include "ant/base/macros.h"
#include "ant/base/ref_counted.h"

namespace ant {

class MetricEntity;

void InitSpinLockContentionProfiling();
uint64_t GetSpinLockContentionMicros();
uint64_t GetTcmallocContentionMicros();
void RegisterSpinLockContentionMetrics(const scoped_refptr<MetricEntity>& entity);
void StartSynchronizationProfiling();
void FlushSynchronizationProfile(std::ostringstream* out, int64_t* drop_count);
void StopSynchronizationProfiling();

} // namespace ant
#endif /* KUDU_UTIL_SPINLOCK_PROFILING_H */
