// Ant Metrics
//
// 关键概念:
//
// Metric Prototypes
// -----------------
// prototype 构造出 metric.
// prototype 定义了metric名字, metric所依附的entity, metric类型, metric单位, 以及一个描述.
//
// 
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

// 定义 MetricEntityPrototype
// MetricEntityPrototype METRIC_ENTITY_test_entity("test_entity");
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

// Counter
// ant::CounterPrototype METRIC_reqs_pending(
//   ::ant::MetricPrototype::CtorArgs("test_entity",
//                                    "reqs_pending",
//                                    "Requests Pending",
//                                    MetricUnit::kRequests,
//                                    "Number of requests pending"))
//
METRIC_DEFINE_counter(test_entity,           // entity
                      reqs_pending,          // name
                      "Requests Pending",    // label
                      MetricUnit::kRequests, // unit
                      "Number of requests pending");

TEST_F(MetricsTest, SimpleCounterTest) {
  // 创建计数器实例
  scoped_refptr<Counter> requests = new Counter(&METRIC_reqs_pending);
  ASSERT_EQ("Number of requests pending", requests->prototype()->description());
  ASSERT_EQ(0, requests->value());
  requests->Increment();
  ASSERT_EQ(1, requests->value());
  requests->IncrementBy(2);
  ASSERT_EQ(3, requests->value());
}

// Gauge
// ant::GaugePrototype<uint64_t> METRIC_fake_memory_usage(
//   ::ant::MetricPrototype::CtorArgs("test_entity",
//                                    "fake_memory_usage",
//                                    "Memory Usage",
//                                    MetricUnit::kBytes,
//                                    "Memory Usage Gauge"));
METRIC_DEFINE_gauge_uint64(test_entity,
                           fake_memory_usage, "Memory Usage",
                           MetricUnit::kBytes, "Memory Usage Gauge");
TEST_F(MetricsTest, SimpleAtomicGaugeTest) {
  // 创建仪表实例
  scoped_refptr<AtomicGauge<uint64_t>> mem_usage = METRIC_fake_memory_usage.Instantiate(entity_, 0);

  ASSERT_EQ(METRIC_fake_memory_usage.description(),
            mem_usage->prototype()->description());
  ASSERT_EQ(0, mem_usage->value());
  mem_usage->IncrementBy(7);
  ASSERT_EQ(7, mem_usage->value());
  mem_usage->set_value(5);
  ASSERT_EQ(5, mem_usage->value());
}

// Histogram
// ant::HistogramPrototype METRIC_test_hits(
//   ::ant::MetricPrototype::CtorArgs("test_entity",
//                                    "test_hist",
//                                    "Test Historgram",
//                                    MetricUnit::kMilliseconds,
//                                    "foo",
//                                    1000 * 1000,
//                                    3));
METRIC_DEFINE_histogram(test_entity,        // Entity
                        test_hist,          // name
                        "Test Histogram",   // label
                        MetricUnit::kMilliseconds, // unit
                        "foo",              // desc
                        1000000,            // max_value, num_sig_digits
                        3);
TEST_F(MetricsTest, SimpleHistogramTest) {
  // 创建直方图实例
  scoped_refptr<Histogram> hist = METRIC_test_hist.Instantiate(entity_);
  hist->Increment(2);
  hist->IncrementBy(4, 1);
  ASSERT_EQ(2, hist->histogram_->MinValue());
  ASSERT_EQ(3, hist->histogram_->MeanValue());
  ASSERT_EQ(4, hist->histogram_->MaxValue());
  ASSERT_EQ(2, hist->histogram_->TotalCount());
  ASSERT_EQ(6, hist->histogram_->TotalSum());
}

// JSON
TEST_F(MetricsTest, JsonPrintTest) {
  scoped_refptr<Counter> bytes_seen = METRIC_reqs_pending.Instantiate(entity_);
  bytes_seen->Increment();
  entity_->SetAttribute("test_attr", "attr_val");

  // 创建 JSON
  std::ostringstream out;
  JsonWriter writer(&out, JsonWriter::PRETTY);
  ASSERT_OK(entity_->WriteAsJson(&writer, {"*"}, MetricJsonOptions()));

  JsonReader reader(out.str());
  ASSERT_OK(reader.Init());

  LOG(INFO) << "JSON: " << out.str();


  vector<const rapidjson::Value*> metrics;
  ASSERT_OK(reader.ExtractObjectArray(reader.root(), "metrics", &metrics));
  ASSERT_EQ(1, metrics.size());
  string metric_name;
  ASSERT_OK(reader.ExtractString(metrics[0], "name", &metric_name));
  ASSERT_EQ("reqs_pending", metric_name);
  int64_t metric_value;
  ASSERT_OK(reader.ExtractInt64(metrics[0], "value", &metric_value));
  ASSERT_EQ(1L, metric_value);

  const rapidjson::Value* attributes;
  ASSERT_OK(reader.ExtractObject(reader.root(), "attributes", &attributes));
  string attr_value;
  ASSERT_OK(reader.ExtractString(attributes, "test_attr", &attr_value));
  ASSERT_EQ("attr_val", attr_value);

  out.str("");
  ASSERT_OK(entity_->WriteAsJson(&writer, { "not_a_matching_metric" }, MetricJsonOptions()));
  ASSERT_EQ("", out.str());
}

// Retirement
TEST_F(MetricsTest, RetirementTest) {
  FLAGS_metrics_retirement_age_ms = 100;

  const std::string kMetricName = "foo";

}

//////////////////


} // namespace ant
