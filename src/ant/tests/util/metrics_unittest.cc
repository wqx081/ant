#include "ant/util/metrics.h"
#include "ant/util/test_util.h"

#include "ant/util/jsonreader.h"
#include "ant/util/jsonwriter.h"

#include "ant/util/hdr_histogram.h"

#include "ant/base/bind.h"
#include "ant/base/map-util.h"

#include <string>
#include <unordered_set>
#include <vector>

DECLARE_int32(metrics_retirement_age_ms);

namespace ant {

METRIC_DEFINE_entity(test_entity);

class MetricsTest : public AntTest {
 public:

  void SetUp() override {
    AntTest::SetUp();
    entity_ = METRIC_ENTITY_test_entity.Instantiate(&registry_, "my-test");
  }

 protected:
  MetricRegistry registry_;
  scoped_refptr<MetricEntity> entity_;
};

METRIC_DEFINE_counter(test_entity, 
                      reqs_pending, "Requests Pending", 
                      MetricUnit::kRequests, "Number of requests pending");
                      
TEST_F(MetricsTest, SimpleCounterTest) {
  scoped_refptr<Counter> requests =
      new Counter(&METRIC_reqs_pending);
  ASSERT_EQ("Number of requests pending", requests->prototype()->description());
  ASSERT_EQ(0, requests->value());
  requests->Increment();
  ASSERT_EQ(1, requests->value());
  requests->IncrementBy(2);
  ASSERT_EQ(3, requests->value());
}


} // namespace ant

