#ifndef KUDU_UTIL_PATH_UTIL_H
#define KUDU_UTIL_PATH_UTIL_H

#include <string>

namespace ant {

// Join two path segments with the appropriate path separator,
// if necessary.
std::string JoinPathSegments(const std::string &a,
                             const std::string &b);

// Return the enclosing directory of path.
// This is like dirname(3) but for C++ strings.
std::string DirName(const std::string& path);

// Return the terminal component of a path.
// This is like basename(3) but for C++ strings.
std::string BaseName(const std::string& path);

} // namespace ant
#endif /* KUDU_UTIL_PATH_UTIL_H */
