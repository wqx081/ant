#include "ant/util/random_util.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

#include "ant/util/env.h"
#include "ant/util/random.h"
#include "ant/base/walltime.h"

namespace ant {

void RandomString(void* dest, size_t n, Random* rng) {
  size_t i = 0;
  uint32_t random = rng->Next();
  char* cdest = static_cast<char*>(dest);
  static const size_t sz = sizeof(random);
  if (n >= sz) {
    for (i = 0; i <= n - sz; i += sz) {
      memcpy(&cdest[i], &random, sizeof(random));
      random = rng->Next();
    }
  }
  memcpy(cdest + i, &random, n - i);
}

uint32_t GetRandomSeed32() {
  uint32_t seed = static_cast<uint32_t>(GetCurrentTimeMicros());
  seed *= getpid();
  seed *= Env::Default()->gettid();
  return seed;
}

} // namespace ant
