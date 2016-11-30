#ifndef KUDU_MEMORY_OVERWRITE_H
#define KUDU_MEMORY_OVERWRITE_H

#include "ant/base/strings/stringpiece.h"

namespace ant {

// Overwrite 'p' with enough repetitions of 'pattern' to fill 'len'
// bytes. This is optimized at -O3 even in debug builds, so is
// reasonably efficient to use.
void OverwriteWithPattern(char* p, size_t len, StringPiece pattern);

} // namespace ant
#endif /* KUDU_MEMORY_OVERWRITE_H */

