#ifndef ANT_UTIL_LOGGING_H
#define ANT_UTIL_LOGGING_H

#include <string>
#include <glog/logging.h>

#include "ant/base/atomicops.h"
#include "ant/base/dynamic_annotations.h"
#include "ant/base/macros.h"
#include "ant/base/walltime.h"
#include "ant/util/logging_callback.h"

////////////////////////////////////////////////////////////////////////////////
// Throttled logging support
////////////////////////////////////////////////////////////////////////////////

// Logs a message throttled to appear at most once every 'n_secs' seconds to
// the given severity.
//
// The log message may include the special token 'THROTTLE_MSG' which expands
// to either an empty string or '[suppressed <n> similar messages]'.
//
// Example usage:
//   KLOG_EVERY_N_SECS(WARNING, 1) << "server is low on memory" << THROTTLE_MSG;
//
//
// Advanced per-instance throttling
// -----------------------------------
// For cases where the throttling should be scoped to a given class instance,
// you may define a logging::LogThrottler object and pass it to the
// KLOG_EVERY_N_SECS_THROTTLER(...) macro. In addition, you must pass a "tag".
// Only log messages with equal tags (by pointer equality) will be throttled.
// For example:
//
//    struct MyThing {
//      string name;
//      LogThrottler throttler;
//    };
//
//    if (...) {
//      LOG_EVERY_N_SECS_THROTTLER(INFO, 1, my_thing->throttler, "coffee") <<
//        my_thing->name << " needs coffee!";
//    } else {
//      LOG_EVERY_N_SECS_THROTTLER(INFO, 1, my_thing->throttler, "wine") <<
//        my_thing->name << " needs wine!";
//    }
//
// In this example, the "coffee"-related message will be collapsed into other
// such messages within the prior one second; however, if the state alternates
// between the "coffee" message and the "wine" message, then each such alternation
// will yield a message.

#define KLOG_EVERY_N_SECS_THROTTLER(severity, n_secs, throttler, tag) \
  int VARNAME_LINENUM(num_suppressed) = 0;                            \
  if (throttler.ShouldLog(n_secs, tag, &VARNAME_LINENUM(num_suppressed)))  \
    google::LogMessage( \
      __FILE__, __LINE__, google::GLOG_ ## severity, VARNAME_LINENUM(num_suppressed), \
      &google::LogMessage::SendToLog).stream()

#define KLOG_EVERY_N_SECS(severity, n_secs) \
  static logging::LogThrottler LOG_THROTTLER;  \
  KLOG_EVERY_N_SECS_THROTTLER(severity, n_secs, LOG_THROTTLER, "no-tag")


namespace ant {
enum PRIVATE_ThrottleMsg {THROTTLE_MSG};
} // namespace ant

////////////////////////////////////////////////////////////////////////////////
// Versions of glog macros for "LOG_EVERY" and "LOG_FIRST" that annotate the
// benign races on their internal static variables.
////////////////////////////////////////////////////////////////////////////////

// The "base" macros.
#define ANT_SOME_KIND_OF_LOG_EVERY_N(severity, n, what_to_do) \
  static int LOG_OCCURRENCES = 0, LOG_OCCURRENCES_MOD_N = 0; \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES, "Logging every N is approximate"); \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES_MOD_N, "Logging every N is approximate"); \
  ++LOG_OCCURRENCES; \
  if (++LOG_OCCURRENCES_MOD_N > n) LOG_OCCURRENCES_MOD_N -= n; \
  if (LOG_OCCURRENCES_MOD_N == 1) \
    google::LogMessage( \
        __FILE__, __LINE__, google::GLOG_ ## severity, LOG_OCCURRENCES, \
        &what_to_do).stream()

#define ANT_SOME_KIND_OF_LOG_IF_EVERY_N(severity, condition, n, what_to_do) \
  static int LOG_OCCURRENCES = 0, LOG_OCCURRENCES_MOD_N = 0; \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES, "Logging every N is approximate"); \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES_MOD_N, "Logging every N is approximate"); \
  ++LOG_OCCURRENCES; \
  if (condition && \
      ((LOG_OCCURRENCES_MOD_N=(LOG_OCCURRENCES_MOD_N + 1) % n) == (1 % n))) \
    google::LogMessage( \
        __FILE__, __LINE__, google::GLOG_ ## severity, LOG_OCCURRENCES, \
                 &what_to_do).stream()

#define ANT_SOME_KIND_OF_PLOG_EVERY_N(severity, n, what_to_do) \
  static int LOG_OCCURRENCES = 0, LOG_OCCURRENCES_MOD_N = 0; \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES, "Logging every N is approximate"); \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES_MOD_N, "Logging every N is approximate"); \
  ++LOG_OCCURRENCES; \
  if (++LOG_OCCURRENCES_MOD_N > n) LOG_OCCURRENCES_MOD_N -= n; \
  if (LOG_OCCURRENCES_MOD_N == 1) \
    google::ErrnoLogMessage( \
        __FILE__, __LINE__, google::GLOG_ ## severity, LOG_OCCURRENCES, \
        &what_to_do).stream()

#define ANT_SOME_KIND_OF_LOG_FIRST_N(severity, n, what_to_do) \
  static uint64_t LOG_OCCURRENCES = 0; \
  ANNOTATE_BENIGN_RACE(&LOG_OCCURRENCES, "Logging the first N is approximate"); \
  if (LOG_OCCURRENCES++ < n) \
    google::LogMessage( \
      __FILE__, __LINE__, google::GLOG_ ## severity, LOG_OCCURRENCES, \
      &what_to_do).stream()

// The direct user-facing macros.
#define KLOG_EVERY_N(severity, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(google::GLOG_ ## severity < \
                             google::NUM_SEVERITIES, \
                             INVALID_REQUESTED_LOG_SEVERITY); \
  ANT_SOME_KIND_OF_LOG_EVERY_N(severity, (n), google::LogMessage::SendToLog)

#define KSYSLOG_EVERY_N(severity, n) \
  ANT_SOME_KIND_OF_LOG_EVERY_N(severity, (n), google::LogMessage::SendToSyslogAndLog)

#define KPLOG_EVERY_N(severity, n) \
  ANT_SOME_KIND_OF_PLOG_EVERY_N(severity, (n), google::LogMessage::SendToLog)

#define KLOG_FIRST_N(severity, n) \
  ANT_SOME_KIND_OF_LOG_FIRST_N(severity, (n), google::LogMessage::SendToLog)

#define KLOG_IF_EVERY_N(severity, condition, n) \
  ANT_SOME_KIND_OF_LOG_IF_EVERY_N(severity, (condition), (n), google::LogMessage::SendToLog)

// We also disable the un-annotated glog macros for anyone who includes this header.
#undef LOG_EVERY_N
#define LOG_EVERY_N(severity, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(false, "LOG_EVERY_N is deprecated. Please use KLOG_EVERY_N.")

#undef SYSLOG_EVERY_N
#define SYSLOG_EVERY_N(severity, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(false, "SYSLOG_EVERY_N is deprecated. Please use KSYSLOG_EVERY_N.")

#undef PLOG_EVERY_N
#define PLOG_EVERY_N(severity, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(false, "PLOG_EVERY_N is deprecated. Please use KPLOG_EVERY_N.")

#undef LOG_FIRST_N
#define LOG_FIRST_N(severity, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(false, "LOG_FIRST_N is deprecated. Please use KLOG_FIRST_N.")

#undef LOG_IF_EVERY_N
#define LOG_IF_EVERY_N(severity, condition, n) \
  GOOGLE_GLOG_COMPILE_ASSERT(false, "LOG_IF_EVERY_N is deprecated. Please use KLOG_IF_EVERY_N.")




namespace ant {

// glog doesn't allow multiple invocations of InitGoogleLogging. This method conditionally
// calls InitGoogleLogging only if it hasn't been called before.
//
// It also takes care of installing the google failure signal handler.
void InitGoogleLoggingSafe(const char* arg);

// Like InitGoogleLoggingSafe() but stripped down: no signal handlers are
// installed, regular logging is disabled, and log events of any severity
// will be written to stderr.
//
// These properties make it attractive for us in libraries.
void InitGoogleLoggingSafeBasic(const char* arg);

// Demotes stderr logging to ERROR or higher and registers 'cb' as the
// recipient for all log events.
//
// Subsequent calls to RegisterLoggingCallback no-op (until the callback
// is unregistered with UnregisterLoggingCallback()).
void RegisterLoggingCallback(const LoggingCallback& cb);

// Unregisters a callback previously registered with
// RegisterLoggingCallback() and promotes stderr logging back to all
// severities.
//
// If no callback is registered, this is a no-op.
void UnregisterLoggingCallback();

// Returns the full pathname of the symlink to the most recent log
// file corresponding to this severity
void GetFullLogFilename(google::LogSeverity severity, std::string* filename);

// Shuts down the google logging library. Call before exit to ensure that log files are
// flushed.
void ShutdownLoggingSafe();

// Writes all command-line flags to the log at level INFO.
void LogCommandLineFlags();

namespace logging {

// A LogThrottler instance tracks the throttling state for a particular
// log message.
//
// This is used internally by KLOG_EVERY_N_SECS, but can also be used
// explicitly in conjunction with KLOG_EVERY_N_SECS_THROTTLER. See the
// macro descriptions above for details.
class LogThrottler {
 public:
  LogThrottler() : num_suppressed_(0), last_ts_(0), last_tag_(nullptr) {
    ANNOTATE_BENIGN_RACE_SIZED(this, sizeof(*this), "OK to be sloppy with log throttling");
  }

  bool ShouldLog(int n_secs, const char* tag, int* num_suppressed) {
    MicrosecondsInt64 ts = GetMonoTimeMicros();

    // When we switch tags, we should not show the "suppressed" messages, because
    // in fact it's a different message that we skipped. So, reset it to zero,
    // and always log the new message.
    if (tag != last_tag_) {
      *num_suppressed = num_suppressed_ = 0;
      last_tag_ = tag;
      last_ts_ = ts;
      return true;
    }

    if (ts - last_ts_ < n_secs * 1e6) {
      *num_suppressed = base::subtle::NoBarrier_AtomicIncrement(&num_suppressed_, 1);
      return false;
    }
    last_ts_ = ts;
    *num_suppressed = base::subtle::NoBarrier_AtomicExchange(&num_suppressed_, 0);
    return true;
  }
 private:
  base::subtle::Atomic32 num_suppressed_;
  uint64_t last_ts_;
  const char* last_tag_;
};
} // namespace logging

std::ostream& operator<<(std::ostream &os, const PRIVATE_ThrottleMsg&);

// Convenience macros to prefix log messages with some prefix, these are the unlocked
// versions and should not obtain a lock (if one is required to obtain the prefix).
// There must be a LogPrefixUnlocked()/LogPrefixLocked() method available in the current
// scope in order to use these macros.
#define LOG_WITH_PREFIX_UNLOCKED(severity) LOG(severity) << LogPrefixUnlocked()
#define VLOG_WITH_PREFIX_UNLOCKED(verboselevel) LOG_IF(INFO, VLOG_IS_ON(verboselevel)) \
  << LogPrefixUnlocked()

// Same as the above, but obtain the lock.
#define LOG_WITH_PREFIX(severity) LOG(severity) << LogPrefix()
#define VLOG_WITH_PREFIX(verboselevel) LOG_IF(INFO, VLOG_IS_ON(verboselevel)) \
  << LogPrefix()

} // namespace ant

#endif // ANT_UTIL_LOGGING_H
