#include "ant/util/malloc.h"

#include <malloc.h>

namespace ant {

int64_t ant_malloc_usable_size(const void* obj) {
  return malloc_usable_size(const_cast<void*>(obj));
}

} // namespace ant
