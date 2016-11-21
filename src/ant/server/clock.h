#ifndef ANT_SERVER_CLOCK_H_
#define ANT_SERVER_CLOCK_H_

#include <string>

#include "ant/common/timestamp.h"
#include "ant/base/ref_counted.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"

namespace ant {

class faststring;
class MetricEntity;
class MonoDelta;
class Slice;
class Status;

class Clock : public base::RefCountedThreadSafe<Clock> {
 public:
  virtual Status Init() = 0;
  virtual Timestamp Now() = 0;
  virtual Timestamp NowLatest() = 0;
  virtual Status GetGlobalLatest(Timestamp* t);
//  virtual bool SupportsExternalConsistencyMode(ExternalConsistencyMode mode) = 0;
  virtual bool HasPhysicalComponent();
  virtual Status Update(const Timestamp& to_update) = 0;
  virtual Status WaitUntilAfterLocally(const Timestamp& then,
                                       const MonoTime& deadline) = 0;
  virtual bool IsAfter(Timestamp t) = 0;
  virtual void RegisterMetrics(const scoped_refptr<MetricEntity>& metric_entity) = 0;
  virtual std::string Stringify(Timestamp timestamp) = 0;
  virtual ~Clock() {}
};

} // namespace ant
#endif // ANT_SERVER_CLOCK_H_
