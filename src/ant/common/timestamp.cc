#include "ant/common/timestamp.h"

#include "ant/util/faststring.h"
#include "ant/util/memcmpable_varint.h"
#include "ant/util/slice.h"
#include "ant/util/status.h"
#include "ant/base/strings/substitute.h"
#include "ant/base/mathlimits.h"

namespace ant {

const Timestamp Timestamp::kMin(MathLimits<Timestamp::val_type>::kMin);
const Timestamp Timestamp::kMax(MathLimits<Timestamp::val_type>::kMax);
const Timestamp Timestamp::kInitialTimestamp(MathLimits<Timestamp::val_type>::kMin + 1);
const Timestamp Timestamp::kInvalidTimestamp(MathLimits<Timestamp::val_type>::kMax - 1);

bool Timestamp::DecodeFrom(Slice *input) {
  return GetMemcmpableVarint64(input, &v);
}

void Timestamp::EncodeTo(faststring *dst) const {
  PutMemcmpableVarint64(dst, v);
}

string Timestamp::ToString() const {
  return strings::Substitute("$0", v);
}

uint64_t Timestamp::ToUint64() const {
  return v;
}

void Timestamp::FromUint64(uint64_t value) {
  v = value;
}

}  // namespace ant
