#ifndef KUDU_SERVER_GLOG_METRICS_H
#define KUDU_SERVER_GLOG_METRICS_H

#include "ant/base/macros.h"
#include "ant/base/gscoped_ptr.h"
#include "ant/base/ref_counted.h"

namespace google {
class LogSink;
} // namespace google

namespace ant {
class MetricEntity;

// Attaches GLog metrics to the given entity, for the duration of this
// scoped object's lifetime.
//
// NOTE: the metrics are collected process-wide, not confined to any set of
// threads, etc.
class ScopedGLogMetrics {
 public:
  explicit ScopedGLogMetrics(const scoped_refptr<MetricEntity>& entity);
  ~ScopedGLogMetrics();

 private:
  gscoped_ptr<google::LogSink> sink_;
};


// Registers glog-related metrics.
// This can be called multiple times on different entities, though the resulting
// metrics will be identical, since the GLog tracking is process-wide.
void RegisterGLogMetrics(const scoped_refptr<MetricEntity>& entity);

} // namespace ant
#endif /* KUDU_SERVER_GLOG_METRICS_H */
