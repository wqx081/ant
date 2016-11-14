#ifndef KUDU_UTIL_MUTEX_H
#define KUDU_UTIL_MUTEX_H

#include <pthread.h>
#include <glog/logging.h>
#include <sys/types.h>

#include "ant/base/gscoped_ptr.h"
#include "ant/base/macros.h"

namespace ant {

// A lock built around pthread_mutex_t. Does not allow recursion.
//
// The following checks will be performed in DEBUG mode:
//   Acquire(), TryAcquire() - the lock isn't already held.
//   Release() - the lock is already held by this thread.
//
class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Acquire();
  void Release();
  bool TryAcquire();

  void lock() { Acquire(); }
  void unlock() { Release(); }
  bool try_lock() { return TryAcquire(); }

  void AssertAcquired() const {}

 private:
  friend class ConditionVariable;

  pthread_mutex_t native_handle_;

  DISALLOW_COPY_AND_ASSIGN(Mutex);
};

// A helper class that acquires the given Lock while the MutexLock is in scope.
class MutexLock {
 public:
  struct AlreadyAcquired {};

  // Acquires 'lock' (must be unheld) and wraps around it.
  //
  // Sample usage:
  // {
  //   MutexLock l(lock_); // acquired
  //   ...
  // } // released
  explicit MutexLock(Mutex& lock)
    : lock_(&lock),
      owned_(true) {
    lock_->Acquire();
  }

  // Wraps around 'lock' (must already be held by this thread).
  //
  // Sample usage:
  // {
  //   lock_.Acquire(); // acquired
  //   ...
  //   MutexLock l(lock_, AlreadyAcquired());
  //   ...
  // } // released
  MutexLock(Mutex& lock, const AlreadyAcquired&)
    : lock_(&lock),
      owned_(true) {
    lock_->AssertAcquired();
  }

  void Lock() {
    DCHECK(!owned_);
    lock_->Acquire();
    owned_ = true;
  }

  void Unlock() {
    DCHECK(owned_);
    lock_->AssertAcquired();
    lock_->Release();
    owned_ = false;
  }

  ~MutexLock() {
    if (owned_) {
      Unlock();
    }
  }

  bool OwnsLock() const {
    return owned_;
  }

 private:
  Mutex* lock_;
  bool owned_;
  DISALLOW_COPY_AND_ASSIGN(MutexLock);
};

} // namespace kudu
#endif /* KUDU_UTIL_MUTEX_H */
