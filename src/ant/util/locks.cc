#include "ant/util/locks.h"
#include "ant/util/malloc.h"

namespace ant {

size_t percpu_rwlock::memory_footprint_excluding_this() const {
  return n_cpus_ * sizeof(padded_lock);
}

size_t percpu_rwlock::memory_footprint_including_this() const {
  return ant_malloc_usable_size(this) + memory_footprint_excluding_this();
}

} // namespace ant
