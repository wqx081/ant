#ifndef KUDU_UTIL_USER_H
#define KUDU_UTIL_USER_H

#include <string>

#include "ant/util/status.h"

namespace ant {

// Get current logged-in user with getpwuid_r().
// user name is written to user_name.
Status GetLoggedInUser(std::string* user_name);

} // namespace ant

#endif // KUDU_UTIL_USER_H
