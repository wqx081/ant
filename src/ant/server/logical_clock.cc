#include "ant/server/logical_clock.h"

#include "ant/base/atomicops.h"
#include "ant/base/bind.h"
#include "ant/base/strings/substitute.h"
#include "ant/util/metrics.h"
#include "ant/util/monotime.h"
#include "ant/util/status.h"

namespace ant {

namespace server {

METRIC_DEFINE_gauge_uint64(server, logical_clock_timestamp,
                           "Logical Clock Timestamp",
                           ant::MetricUnit::kUnits,
                           "Logical clock timestamp.");

using base::subtle::Atomic64;
using base::subtle::Barrier_AtomicIncrement;
using base::subtle::NoBarrier_CompareAndSwap;

Timestamp LogicalClock::Now() {
  return Timestamp(Barrier_AtomicIncrement(&now_, 1));
}

Timestamp LogicalClock::NowLatest() {
  return Now();
}

Status LogicalClock::Update(const Timestamp& to_update) {
  DCHECK_NE(to_update.value(), Timestamp::kInvalidTimestamp.value())
      << "Updating the clock with an invalid timestamp";
  Atomic64 new_value = to_update.value();

  while (true) {
    Atomic64 current_value = NoBarrier_Load(&now_);
    // if the incoming value is less than the current one, or we've failed the
    // CAS because the current clock increased to higher than the incoming value,
    // we can stop the loop now.
    if (new_value <= current_value) return Status::OK();
    // otherwise try a CAS
    if (PREDICT_TRUE(NoBarrier_CompareAndSwap(&now_, current_value, new_value)
        == current_value))
      break;
  }
  return Status::OK();
}

Status LogicalClock::WaitUntilAfter(const Timestamp& then,
                                    const MonoTime& deadline) {
  return Status::ServiceUnavailable(
      "Logical clock does not support WaitUntilAfter()");
}

Status LogicalClock::WaitUntilAfterLocally(const Timestamp& then,
                                           const MonoTime& deadline) {
  if (IsAfter(then)) return Status::OK();
  return Status::ServiceUnavailable(
      "Logical clock does not support WaitUntilAfterLocally()");
}

bool LogicalClock::IsAfter(Timestamp t) {
  return base::subtle::Acquire_Load(&now_) >= t.value();
}

LogicalClock* LogicalClock::CreateStartingAt(const Timestamp& timestamp) {
  // initialize at 'timestamp' - 1 so that the  first output value is 'timestamp'.
  return new LogicalClock(timestamp.value() - 1);
}

uint64_t LogicalClock::GetCurrentTime() {
  // We don't want reading metrics to change the clock.
  return NoBarrier_Load(&now_);
}

void LogicalClock::RegisterMetrics(const scoped_refptr<MetricEntity>& metric_entity) {
  METRIC_logical_clock_timestamp.InstantiateFunctionGauge(
      metric_entity,
      Bind(&LogicalClock::GetCurrentTime, Unretained(this)))
    ->AutoDetachToLastValue(&metric_detacher_);
}

string LogicalClock::Stringify(Timestamp timestamp) {
  return strings::Substitute("L: $0", timestamp.ToUint64());
}

}  // namespace server
}  // namespace ant

