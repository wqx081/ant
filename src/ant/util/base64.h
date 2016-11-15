#ifndef MCS_BASE_STRINGS_BASE64_H_
#define MCS_BASE_STRINGS_BASE64_H_

#include <string>

#include "ant/util/status.h"
#include "ant/base/strings/stringpiece.h"

namespace ant {

Status Base64Encode(StringPiece data, bool with_padding, std::string* encoded);
Status Base64Encode(StringPiece data, std::string* encoded);  // with_padding=false.

Status Base64Decode(StringPiece data, std::string* decoded);

}  // namespace ant 

#endif // MCS_BASE_STRINGS_BASE64_H_
