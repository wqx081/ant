#ifndef KUDU_UTIL_TRACE_H
#define KUDU_UTIL_TRACE_H

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include "ant/base/macros.h"
#include "ant/base/strings/stringpiece.h"
#include "ant/base/strings/substitute.h"
#include "ant/base/gscoped_ptr.h"
#include "ant/base/ref_counted.h"
#include "ant/base/threading/thread_collision_warner.h"
#include "ant/base/walltime.h"
#include "ant/util/locks.h"
#include "ant/util/trace_metrics.h"

// Adopt a Trace on the current thread for the duration of the current
// scope. The old current Trace is restored when the scope is exited.
//
// 't' should be a Trace* pointer.
#define ADOPT_TRACE(t) ant::ScopedAdoptTrace _adopt_trace(t);

// Issue a trace message, if tracing is enabled in the current thread.
// See Trace::SubstituteAndTrace for arguments.
// Example:
//  TRACE("Acquired timestamp $0", timestamp);
#define TRACE(format, substitutions...) \
  do { \
    ant::Trace* _trace = Trace::CurrentTrace(); \
    if (_trace) { \
      _trace->SubstituteAndTrace(__FILE__, __LINE__, (format),  \
        ##substitutions); \
    } \
  } while (0);

// Like the above, but takes the trace pointer as an explicit argument.
#define TRACE_TO(trace, format, substitutions...) \
  (trace)->SubstituteAndTrace(__FILE__, __LINE__, (format), ##substitutions)

// Increment a counter associated with the current trace.
//
// Each trace contains a map of counters which can be used to keep
// request-specific statistics. It is significantly faster to increment
// a trace counter compared to logging a message. Additionally, having
// slightly more structured information makes it easier to aggregate
// and show information back to operators.
//
// NOTE: the 'counter_name' MUST be a string which stays alive forever.
// Typically, this is a compile-time constant. If something other than
// a constant is required, use TraceMetric::InternName() in order to
// create a string which will last for the process lifetime. Of course,
// these strings will never be cleaned up, so it's important to use this
// judiciously.
//
// If no trace is active, this does nothing and does not evaluate its
// parameters.
#define TRACE_COUNTER_INCREMENT(counter_name, val) \
  do { \
    ant::Trace* _trace = Trace::CurrentTrace(); \
    if (_trace) { \
      _trace->metrics()->Increment(counter_name, val); \
    } \
  } while (0);

// Increment a counter for the amount of wall time spent in the current
// scope. For example:
//
//  void DoFoo() {
//    TRACE_COUNTER_SCOPE_LATENCY_US("foo_us");
//    ... do expensive Foo thing
//  }
//
//  will result in a trace metric indicating the number of microseconds spent
//  in invocations of DoFoo().
#define TRACE_COUNTER_SCOPE_LATENCY_US(counter_name) \
  ::ant::ScopedTraceLatencyCounter _scoped_latency(counter_name)

// Construct a constant C string counter name which acts as a sort of
// coarse-grained histogram for trace metrics.
#define BUCKETED_COUNTER_NAME(prefix, duration_us)      \
  [=]() -> const char* {                                \
    if (duration_us >= 100 * 1000) {                    \
      return prefix "_gt_100_ms";                       \
    } else if (duration_us >= 10 * 1000) {              \
      return prefix "_10-100_ms";                       \
    } else if (duration_us >= 1000) {                   \
      return prefix "_1-10_ms";                         \
    } else {                                            \
      return prefix "_lt_1ms";                          \
    }                                                   \
  }();

namespace ant {

class JsonWriter;
class ThreadSafeArena;
struct TraceEntry;

// A trace for a request or other process. This supports collecting trace entries
// from a number of threads, and later dumping the results to a stream.
//
// Callers should generally not add trace messages directly using the public
// methods of this class. Rather, the TRACE(...) macros defined above should
// be used such that file/line numbers are automatically included, etc.
//
// This class is thread-safe.
class Trace : public base::RefCountedThreadSafe<Trace> {
 public:
  Trace();

  // Logs a message into the trace buffer.
  //
  // See strings::Substitute for details.
  //
  // N.B.: the file path passed here is not copied, so should be a static
  // constant (eg __FILE__).
  void SubstituteAndTrace(const char* filepath, int line_number,
                          StringPiece format,
                          const strings::internal::SubstituteArg& arg0 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg1 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg2 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg3 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg4 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg5 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg6 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg7 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg8 =
                            strings::internal::SubstituteArg::NoArg,
                          const strings::internal::SubstituteArg& arg9 =
                            strings::internal::SubstituteArg::NoArg);

  // Dump the trace buffer to the given output stream.
  //
  enum {
    NO_FLAGS = 0,

    // If set, calculate and print the difference between successive trace messages.
    INCLUDE_TIME_DELTAS = 1 << 0,
    // If set, include a 'Metrics' line showing any attached trace metrics.
    INCLUDE_METRICS =     1 << 1,

    INCLUDE_ALL = INCLUDE_TIME_DELTAS | INCLUDE_METRICS
  };
  void Dump(std::ostream* out, int flags) const;

  // Dump the trace buffer as a string.
  std::string DumpToString(int flags = INCLUDE_ALL) const;

  std::string MetricsAsJSON() const;

  // Attaches the given trace which will get appended at the end when Dumping.
  //
  // The 'label' does not necessarily have to be unique, and is used to identify
  // the child trace when dumped. The contents of the StringPiece are copied
  // into this trace's arena.
  void AddChildTrace(StringPiece label, Trace* child_trace);

  // Return a copy of the current set of related "child" traces.
  std::vector<std::pair<StringPiece, scoped_refptr<Trace>>> ChildTraces() const;

  // Return the current trace attached to this thread, if there is one.
  static Trace* CurrentTrace() {
    return threadlocal_trace_;
  }

  // Simple function to dump the current trace to stderr, if one is
  // available. This is meant for usage when debugging in gdb via
  // 'call ant::Trace::DumpCurrentTrace();'.
  static void DumpCurrentTrace();

  TraceMetrics* metrics() {
    return &metrics_;
  }
  const TraceMetrics& metrics() const {
    return metrics_;
  }

 private:
  friend class ScopedAdoptTrace;
  friend class RefCountedThreadSafe<Trace>;
  ~Trace();

  // The current trace for this thread. Threads should only set this using
  // using ScopedAdoptTrace, which handles reference counting the underlying
  // object.
  static __thread Trace* threadlocal_trace_;

  // Allocate a new entry from the arena, with enough space to hold a
  // message of length 'len'.
  TraceEntry* NewEntry(int len, const char* file_path, int line_number);

  // Add the entry to the linked list of entries.
  void AddEntry(TraceEntry* entry);

  void MetricsToJSON(JsonWriter* jw) const;

  gscoped_ptr<ThreadSafeArena> arena_;

  // Lock protecting the entries linked list.
  mutable simple_spinlock lock_;
  // The head of the linked list of entries (allocated inside arena_)
  TraceEntry* entries_head_;
  // The tail of the linked list of entries (allocated inside arena_)
  TraceEntry* entries_tail_;

  std::vector<std::pair<StringPiece, scoped_refptr<Trace>>> child_traces_;

  TraceMetrics metrics_;

  DISALLOW_COPY_AND_ASSIGN(Trace);
};

// Adopt a Trace object into the current thread for the duration
// of this object.
// This should only be used on the stack (and thus created and destroyed
// on the same thread)
class ScopedAdoptTrace {
 public:
  explicit ScopedAdoptTrace(Trace* t) :
    old_trace_(Trace::threadlocal_trace_) {
    Trace::threadlocal_trace_ = t;
    if (t) {
      t->AddRef();
    }
    DFAKE_SCOPED_LOCK_THREAD_LOCKED(ctor_dtor_);
  }

  ~ScopedAdoptTrace() {
    auto t = Trace::threadlocal_trace_;
    Trace::threadlocal_trace_ = old_trace_;

    // It's critical that we Release() the reference count on 't' only
    // after we've unset the thread-local variable. Otherwise, we can hit
    // a nasty interaction with tcmalloc contention profiling. Consider
    // the following sequence:
    //
    //   1. threadlocal_trace_ has refcount = 1
    //   2. we call threadlocal_trace_->Release() which decrements refcount to 0
    //   3. this calls 'delete' on the Trace object
    //   3a. this calls tcmalloc free() on the Trace and various sub-objects
    //   3b. the free() calls may end up experiencing contention in tcmalloc
    //   3c. we try to account the contention in threadlocal_trace_'s TraceMetrics,
    //       but it has already been freed.
    //
    // In the best case, we just scribble into some free tcmalloc memory. In the
    // worst case, tcmalloc would have already re-used this memory for a new
    // allocation on another thread, and we end up overwriting someone else's memory.
    //
    // Waiting to Release() only after 'unpublishing' the trace solves this.
    if (t) {
      t->Release();
    }
    DFAKE_SCOPED_LOCK_THREAD_LOCKED(ctor_dtor_);
  }

 private:
  DFAKE_MUTEX(ctor_dtor_);
  Trace* old_trace_;

  DISALLOW_COPY_AND_ASSIGN(ScopedAdoptTrace);
};

// Implementation for TRACE_COUNTER_SCOPE_LATENCY_US(...) macro above.
class ScopedTraceLatencyCounter {
 public:
  explicit ScopedTraceLatencyCounter(const char* counter)
      : counter_(counter),
        start_time_(GetCurrentTimeMicros()) {
  }

  ~ScopedTraceLatencyCounter() {
    TRACE_COUNTER_INCREMENT(counter_, GetCurrentTimeMicros() - start_time_);
  }

 private:
  const char* const counter_;
  MicrosecondsInt64 start_time_;
  DISALLOW_COPY_AND_ASSIGN(ScopedTraceLatencyCounter);
};

} // namespace ant
#endif /* KUDU_UTIL_TRACE_H */
