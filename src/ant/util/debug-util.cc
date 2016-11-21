#include "ant/util/debug-util.h"

#include <execinfo.h>
#include <dirent.h>
#include <glog/logging.h>
#include <signal.h>
#include <string>
#include <sys/syscall.h>

#include "ant/base/macros.h"
#include "ant/base/spinlock.h"
#include "ant/base/stringprintf.h"
#include "ant/base/strings/numbers.h"
//TODO(wqx)
//#include "ant/util/debug/sanitizer_scopes.h"
#include "ant/util/env.h"
#include "ant/util/errno.h"
#include "ant/util/monotime.h"
#include "ant/util/thread.h"


using namespace base::subtle;

extern "C" {
__attribute__((weak))
void __gcov_flush();
}

// Evil hack to grab a few useful functions from glog
namespace google {

extern int GetStackTrace(void** result, int max_depth, int skip_count);

// Symbolizes a program counter.  On success, returns true and write the
// symbol name to "out".  The symbol name is demangled if possible
// (supports symbols generated by GCC 3.x or newer).  Otherwise,
// returns false.
bool Symbolize(void *pc, char *out, int out_size);

namespace glog_internal_namespace_ {
extern void DumpStackTraceToString(std::string *s);
} // namespace glog_internal_namespace_
} // namespace google

// The %p field width for printf() functions is two characters per byte.
// For some environments, add two extra bytes for the leading "0x".
static const int kPrintfPointerFieldWidth = 2 + 2 * sizeof(void*);

// The signal that we'll use to communicate with our other threads.
// This can't be in used by other libraries in the process.
static int g_stack_trace_signum = SIGUSR2;

// We only allow a single dumper thread to run at a time. This simplifies the synchronization
// between the dumper and the target thread.
//
// This lock also protects changes to the signal handler.
static base::SpinLock g_dumper_thread_lock(base::LINKER_INITIALIZED);

namespace ant {

bool IsCoverageBuild() {
  return __gcov_flush != nullptr;
}

void TryFlushCoverage() {
  static base::SpinLock lock(base::LINKER_INITIALIZED);

  // Flushing coverage is not reentrant or thread-safe.
  if (!__gcov_flush || !lock.TryLock()) {
    return;
  }

  __gcov_flush();

  lock.Unlock();
}



namespace {

// Global structure used to communicate between the signal handler
// and a dumping thread.
struct SignalCommunication {
  // The actual stack trace collected from the target thread.
  StackTrace stack;

  // The current target. Signals can be delivered asynchronously, so the
  // dumper thread sets this variable first before sending a signal. If
  // a signal is received on a thread that doesn't match 'target_tid', it is
  // ignored.
  pid_t target_tid;

  // Set to 1 when the target thread has successfully collected its stack.
  // The dumper thread spins waiting for this to become true.
  Atomic32 result_ready;

  // Lock protecting the other members. We use a bare atomic here and a custom
  // lock guard below instead of existing spinlock implementaitons because futex()
  // is not signal-safe.
  Atomic32 lock;

  struct Lock;
};
SignalCommunication g_comm;

// Pared-down SpinLock for SignalCommunication::lock. This doesn't rely on futex
// so it is async-signal safe.
struct SignalCommunication::Lock {
  Lock() {
    while (base::subtle::Acquire_CompareAndSwap(&g_comm.lock, 0, 1) != 0) {
      sched_yield();
    }
  }
  ~Lock() {
    base::subtle::Release_Store(&g_comm.lock, 0);
  }
};

// Signal handler for our stack trace signal.
// We expect that the signal is only sent from DumpThreadStack() -- not by a user.
void HandleStackTraceSignal(int signum) {
  SignalCommunication::Lock l;

  // Check that the dumper thread is still interested in our stack trace.
  // It's possible for signal delivery to be artificially delayed, in which
  // case the dumper thread would have already timed out and moved on with
  // its life. In that case, we don't want to race with some other thread's
  // dump.
  int64_t my_tid = Thread::CurrentThreadId();
  if (g_comm.target_tid != my_tid) {
    return;
  }

  g_comm.stack.Collect(2);
  base::subtle::Release_Store(&g_comm.result_ready, 1);
}

bool InitSignalHandlerUnlocked(int signum) {
  enum InitState {
    UNINITIALIZED,
    INIT_ERROR,
    INITIALIZED
  };
  static InitState state = UNINITIALIZED;

  // If we've already registered a handler, but we're being asked to
  // change our signal, unregister the old one.
  if (signum != g_stack_trace_signum && state == INITIALIZED) {
    struct sigaction old_act;
    PCHECK(sigaction(g_stack_trace_signum, nullptr, &old_act) == 0);
    if (old_act.sa_handler == &HandleStackTraceSignal) {
      signal(g_stack_trace_signum, SIG_DFL);
    }
  }

  // If we'd previously had an error, but the signal number
  // is changing, we should mark ourselves uninitialized.
  if (signum != g_stack_trace_signum) {
    g_stack_trace_signum = signum;
    state = UNINITIALIZED;
  }

  if (state == UNINITIALIZED) {
    struct sigaction old_act;
    PCHECK(sigaction(g_stack_trace_signum, nullptr, &old_act) == 0);
    if (old_act.sa_handler != SIG_DFL &&
        old_act.sa_handler != SIG_IGN) {
      state = INIT_ERROR;
      LOG(WARNING) << "signal handler for stack trace signal "
                   << g_stack_trace_signum
                   << " is already in use: "
                   << "Kudu will not produce thread stack traces.";
    } else {
      // No one appears to be using the signal. This is racy, but there is no
      // atomic swap capability.
      sighandler_t old_handler = signal(g_stack_trace_signum, HandleStackTraceSignal);
      if (old_handler != SIG_IGN &&
          old_handler != SIG_DFL) {
        LOG(FATAL) << "raced against another thread installing a signal handler";
      }
      state = INITIALIZED;
    }
  }
  return state == INITIALIZED;
}

} // namespace

Status SetStackTraceSignal(int signum) {
  base::SpinLockHolder h(&g_dumper_thread_lock);
  if (!InitSignalHandlerUnlocked(signum)) {
    return Status::InvalidArgument("unable to install signal handler");
  }
  return Status::OK();
}

std::string DumpThreadStack(int64_t tid) {
  base::SpinLockHolder h(&g_dumper_thread_lock);

  // Ensure that our signal handler is installed. We don't need any fancy GoogleOnce here
  // because of the mutex above.
  if (!InitSignalHandlerUnlocked(g_stack_trace_signum)) {
    return "<unable to take thread stack: signal handler unavailable>";
  }

  // Set the target TID in our communication structure, so if we end up with any
  // delayed signal reaching some other thread, it will know to ignore it.
  {
    SignalCommunication::Lock l;
    CHECK_EQ(0, g_comm.target_tid);
    g_comm.target_tid = tid;
  }

  // We use the raw syscall here instead of kill() to ensure that we don't accidentally
  // send a signal to some other process in the case that the thread has exited and
  // the TID been recycled.
  if (syscall(SYS_tgkill, getpid(), tid, g_stack_trace_signum) != 0) {
    {
      SignalCommunication::Lock l;
      g_comm.target_tid = 0;
    }
    return "(unable to deliver signal: process may have exited)";
  }

  // We give the thread ~1s to respond. In testing, threads typically respond within
  // a few iterations of the loop, so this timeout is very conservative.
  //
  // The main reason that a thread would not respond is that it has blocked signals. For
  // example, glibc's timer_thread doesn't respond to our signal, so we always time out
  // on that one.
  string ret;
  int i = 0;
  while (!base::subtle::Acquire_Load(&g_comm.result_ready) &&
         i++ < 100) {
    SleepFor(MonoDelta::FromMilliseconds(10));
  }

  {
    SignalCommunication::Lock l;
    CHECK_EQ(tid, g_comm.target_tid);

    if (!g_comm.result_ready) {
      ret = "(thread did not respond: maybe it is blocking signals)";
    } else {
      ret = g_comm.stack.Symbolize();
    }

    g_comm.target_tid = 0;
    g_comm.result_ready = 0;
  }
  return ret;
}

Status ListThreads(vector<pid_t> *tids) {
  DIR *dir = opendir("/proc/self/task/");
  if (dir == NULL) {
    return Status::IOError("failed to open task dir", ErrnoToString(errno), errno);
  }
  struct dirent *d;
  while ((d = readdir(dir)) != NULL) {
    if (d->d_name[0] != '.') {
      uint32_t tid;
      if (!safe_strtou32(d->d_name, &tid)) {
        LOG(WARNING) << "bad tid found in procfs: " << d->d_name;
        continue;
      }
      tids->push_back(tid);
    }
  }
  closedir(dir);
  return Status::OK();
}

std::string GetStackTrace() {
  std::string s;
  google::glog_internal_namespace_::DumpStackTraceToString(&s);
  return s;
}

std::string GetStackTraceHex() {
  char buf[1024];
  HexStackTraceToString(buf, 1024);
  return std::string(buf);
}

void HexStackTraceToString(char* buf, size_t size) {
  StackTrace trace;
  trace.Collect(1);
  trace.StringifyToHex(buf, size);
}

string GetLogFormatStackTraceHex() {
  StackTrace trace;
  trace.Collect(1);
  return trace.ToLogFormatHexString();
}

void StackTrace::Collect(int skip_frames) {
  // google::GetStackTrace has a data race. This is called frequently, so better
  // to ignore it with an annotation rather than use a suppression.
  //TODO(wqx):
  //debug::ScopedTSANIgnoreReadsAndWrites ignore_tsan;
  num_frames_ = google::GetStackTrace(frames_, arraysize(frames_), skip_frames);
}

void StackTrace::StringifyToHex(char* buf, size_t size, int flags) const {
  char* dst = buf;

  // Reserve kHexEntryLength for the first iteration of the loop, 1 byte for a
  // space (which we may not need if there's just one frame), and 1 for a nul
  // terminator.
  char* limit = dst + size - kHexEntryLength - 2;
  for (int i = 0; i < num_frames_ && dst < limit; i++) {
    if (i != 0) {
      *dst++ = ' ';
    }
    // See note in Symbolize() below about why we subtract 1 from each address here.
    uintptr_t addr = reinterpret_cast<uintptr_t>(frames_[i]);
    if (!(flags & NO_FIX_CALLER_ADDRESSES)) {
      addr--;
    }
    FastHex64ToBuffer(addr, dst);
    dst += kHexEntryLength;
  }
  *dst = '\0';
}

string StackTrace::ToHexString(int flags) const {
  // Each frame requires kHexEntryLength, plus a space
  // We also need one more byte at the end for '\0'
  char buf[kMaxFrames * (kHexEntryLength + 1) + 1];
  StringifyToHex(buf, arraysize(buf), flags);
  return string(buf);
}

// Symbolization function borrowed from glog.
string StackTrace::Symbolize() const {
  string ret;
  for (int i = 0; i < num_frames_; i++) {
    void* pc = frames_[i];

    char tmp[1024];
    const char* symbol = "(unknown)";

    // The return address 'pc' on the stack is the address of the instruction
    // following the 'call' instruction. In the case of calling a function annotated
    // 'noreturn', this address may actually be the first instruction of the next
    // function, because the function we care about ends with the 'call'.
    // So, we subtract 1 from 'pc' so that we're pointing at the 'call' instead
    // of the return address.
    //
    // For example, compiling a C program with -O2 that simply calls 'abort()' yields
    // the following disassembly:
    //     Disassembly of section .text:
    //
    //     0000000000400440 <main>:
    //       400440:	48 83 ec 08          	sub    $0x8,%rsp
    //       400444:	e8 c7 ff ff ff       	callq  400410 <abort@plt>
    //
    //     0000000000400449 <_start>:
    //       400449:	31 ed                	xor    %ebp,%ebp
    //       ...
    //
    // If we were to take a stack trace while inside 'abort', the return pointer
    // on the stack would be 0x400449 (the first instruction of '_start'). By subtracting
    // 1, we end up with 0x400448, which is still within 'main'.
    //
    // This also ensures that we point at the correct line number when using addr2line
    // on logged stacks.
    if (google::Symbolize(
            reinterpret_cast<char *>(pc) - 1, tmp, sizeof(tmp))) {
      symbol = tmp;
    }
    StringAppendF(&ret, "    @ %*p  %s\n", kPrintfPointerFieldWidth, pc, symbol);
  }
  return ret;
}

string StackTrace::ToLogFormatHexString() const {
  string ret;
  for (int i = 0; i < num_frames_; i++) {
    void* pc = frames_[i];
    StringAppendF(&ret, "    @ %*p\n", kPrintfPointerFieldWidth, pc);
  }
  return ret;
}

uint64_t StackTrace::HashCode() const {
  return util_hash::CityHash64(reinterpret_cast<const char*>(frames_),
                               sizeof(frames_[0]) * num_frames_);
}

}  // namespace ant
