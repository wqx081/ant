#ifndef KUDU_UTIL_VERSION_INFO_H
#define KUDU_UTIL_VERSION_INFO_H

#include <string>

#include "ant/base/macros.h"

namespace ant {

class VersionInfoPB;

// Static functions related to fetching information about the current build.
class VersionInfo {
 public:
  // Get a short version string ("ant 1.2.3 (rev abcdef...)")
  static std::string GetShortVersionString();

  // Get a multi-line string including version info, build time, etc.
  static std::string GetAllVersionInfo();

  // Set the version info in 'pb'.
  static void GetVersionInfoPB(VersionInfoPB* pb);
 private:
  // Get the git hash for this build. If the working directory was dirty when
  // Kudu was built, also appends "-dirty".
  static std::string GetGitHash();

  DISALLOW_IMPLICIT_CONSTRUCTORS(VersionInfo);
};

} // namespace ant
#endif /* KUDU_UTIL_VERSION_INFO_H */
