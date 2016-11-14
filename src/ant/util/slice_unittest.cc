#include "ant/util/slice.h"
#include "ant/base/map-util.h"

#include <gtest/gtest.h>


using std::string;

namespace ant {

typedef SliceMap<int>::type MySliceMap;

TEST(SliceTest, TestSliceMap) {
  MySliceMap my_map;
  Slice a("a");
  Slice b("b");
  Slice c("c");

  // Insertion is deliberately out-of-order; the map should restore order.
  InsertOrDie(&my_map, c, 3);
  InsertOrDie(&my_map, a, 1);
  InsertOrDie(&my_map, b, 2);

  int expectedValue = 0;
  for (const MySliceMap::value_type& pair : my_map) {
    int data = 'a' + expectedValue++;
    ASSERT_EQ(Slice(reinterpret_cast<uint8_t*>(&data), 1), pair.first);
    ASSERT_EQ(expectedValue, pair.second);
  }

  expectedValue = 0;
  for (auto iter = my_map.begin(); iter != my_map.end(); iter++) {
    int data = 'a' + expectedValue++;
    ASSERT_EQ(Slice(reinterpret_cast<uint8_t*>(&data), 1), iter->first);
    ASSERT_EQ(expectedValue, iter->second);
  }
}

} // namespace kudu
