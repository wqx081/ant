#ifndef KUDU_SERVER_CLOCK_H_
#define KUDU_SERVER_CLOCK_H_

#include <string>

#include "ant/common/common.pb.h"
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

namespace server {

// An interface for a clock that can be used to assign timestamps to
// operations.
// Implementations must respect the following assumptions:
// 1 - Now() must return monotonically increasing numbers
//     i.e. for any two calls, i.e. Now returns timestamp1 and timestamp2, it must
//     hold that timestamp1 < timestamp2.
// 2 - Update() must never set the clock backwards (corollary of 1)
class Clock : public base::RefCountedThreadSafe<Clock> {
 public:

  // Initializes the clock.
  virtual Status Init() = 0;

  // Obtains a new transaction timestamp corresponding to the current instant.
  virtual Timestamp Now() = 0;

  // Obtains a new transaction timestamp corresponding to the current instant
  // plus the max_error.
  virtual Timestamp NowLatest() = 0;

  // Obtain a timestamp which is guaranteed to be later than the current time
  // on any machine in the cluster.
  //
  // NOTE: this is not a very tight bound.
  virtual Status GetGlobalLatest(Timestamp* t) {
    return Status::NotSupported("clock does not support global properties");
  }

  // Indicates whether this clock supports the required external consistency mode.
  virtual bool SupportsExternalConsistencyMode(ExternalConsistencyMode mode) = 0;

  // Indicates whether the clock has a physical component to its timestamps
  // (wallclock time).
  virtual bool HasPhysicalComponent() {
    return false;
  }

  // Update the clock with a transaction timestamp originating from
  // another server. For instance replicas can call this so that,
  // if elected leader, they are guaranteed to generate timestamps
  // higher than the timestamp of the last transaction accepted from the
  // leader.
  virtual Status Update(const Timestamp& to_update) = 0;

  // Waits until the clock on all machines has advanced past 'then'.
  // Can also be used to implement 'external consistency' in the same sense as
  // Google's Spanner.
  virtual Status WaitUntilAfter(const Timestamp& then,
                                const MonoTime& deadline) = 0;

  // Waits until the clock on this machine advances past 'then'. Unlike
  // WaitUntilAfter(), this does not make any global guarantees.
  virtual Status WaitUntilAfterLocally(const Timestamp& then,
                                       const MonoTime& deadline) = 0;

  // Return true if the given time has definitely passed (i.e any future call
  // to Now() would return a higher value than t).
  virtual bool IsAfter(Timestamp t) = 0;

  // Register the clock metrics in the given entity.
  virtual void RegisterMetrics(const scoped_refptr<MetricEntity>& metric_entity) = 0;

  // Strigifies the provided timestamp according to this clock's internal format.
  virtual std::string Stringify(Timestamp timestamp) = 0;

  virtual ~Clock() {}
};

} // namespace server
} // namespace ant

#endif /* KUDU_SERVER_CLOCK_H_ */
