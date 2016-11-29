#include "ant/tests/rpc/rpc-test-base.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include <gtest/gtest.h>

#include "ant/base/map-util.h"
#include "ant/base/strings/join.h"
#include "ant/base/strings/substitute.h"
#include "ant/rpc/constants.h"
#include "ant/rpc/serialization.h"
#include "ant/util/countdown_latch.h"
#include "ant/util/env.h"
#include "ant/util/test_util.h"

#include "ant/util/scoped_cleanup.h"

METRIC_DECLARE_histogram(handler_latency_ant_rpc_test_CalculatorService_Sleep);
METRIC_DECLARE_histogram(rpc_incoming_queue_time);

DECLARE_int32(rpc_negotiation_inject_delay_ms);

using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace ant {
namespace rpc {

class TestRpc : public RpcTestBase,
                public ::testing::WithParamInterface<bool> {
};
INSTANTIATE_TEST_CASE_P(OptionalSSL, TestRpc, testing::Values(false, true));


TEST_F(TestRpc, TestSockaddr) {
  Sockaddr addr1, addr2;
  addr1.set_port(1000);
  addr2.set_port(2000);
  ASSERT_FALSE(addr1 < addr2);
  ASSERT_FALSE(addr2 < addr1);
  ASSERT_EQ(1000, addr1.port());
  ASSERT_EQ(2000, addr2.port());
  ASSERT_EQ("0.0.0.0:1000", addr1.ToString());
  ASSERT_EQ("0.0.0.0:2000", addr2.ToString());
  Sockaddr addr3(addr1);
  ASSERT_EQ(std::string("0.0.0.0:1000"), addr3.ToString());
}

TEST_P(TestRpc, TestMessengerCreateDestroy) {
  shared_ptr<Messenger> messenger(CreateMessenger("TestCreateDestroy", 1, GetParam()));
  LOG(INFO) << "started messenger " << messenger->name();
  messenger->Shutdown();
}

TEST_P(TestRpc, TestAcceptorPoolStartStop) {
  int n_iters = AllowSlowTests() ? 100 : 5;
  for (int i = 0; i < n_iters; i++) {
    shared_ptr<Messenger> messenger(CreateMessenger("TestAcceptorPoolStartStop", 1, GetParam()));
    shared_ptr<AcceptorPool> pool;
    ASSERT_OK(messenger->AddAcceptorPool(Sockaddr(), &pool));
    Sockaddr bound_addr;
    ASSERT_OK(pool->GetBoundAddress(&bound_addr));
    ASSERT_NE(0, bound_addr.port());
    ASSERT_OK(pool->Start(2));
    messenger->Shutdown();
  }
}

TEST_F(TestRpc, TestConnHeaderValidation) {
  MessengerBuilder mb("TestRpc.TestConnHeaderValidation");
  const int conn_hdr_len = kMagicNumberLength + kHeaderFlagsLength;
  uint8_t buf[conn_hdr_len];
  serialization::SerializeConnHeader(buf);
  ASSERT_OK(serialization::ValidateConnHeader(Slice(buf, conn_hdr_len)));
}


TEST_P(TestRpc, TestCall) {
  // setup server
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);

  // setup client
  LOG(INFO) << "Connecting to " << server_addr.ToString();
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  ASSERT_STR_CONTAINS(p.ToString(), strings::Substitute("ant.rpc.GenericCalculatorService@"
                                                         "{remote=$0, user_credentials=",
                                                         server_addr.ToString()));

  for (int i = 0; i < 10; i++) {
    ASSERT_OK(DoTestSyncCall(p, GenericCalculatorService::kAddMethodName));
  }
}

TEST_P(TestRpc, TestCallToBadServer) {
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, GetParam()));
  Sockaddr addr;
  addr.set_port(0);
  Proxy p(client_messenger, addr, GenericCalculatorService::static_service_name());

  for (int i = 0; i < 5; i++) {
    Status s = DoTestSyncCall(p, GenericCalculatorService::kAddMethodName);
    LOG(INFO) << "Status: " << s.ToString();
    ASSERT_TRUE(s.IsNetworkError()) << "unexpected status: " << s.ToString();
  }
}

TEST_P(TestRpc, TestInvalidMethodCall) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);

  LOG(INFO) << "Connecting to " << server_addr.ToString();
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());

  Status s = DoTestSyncCall(p, "ThisMethodDoesNotExist");
  ASSERT_TRUE(s.IsRemoteError()) << "unexpected status: " << s.ToString();
  ASSERT_STR_CONTAINS(s.ToString(), "bad method");
} 

TEST_P(TestRpc, TestWrongService) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, "WrongServiceName");
  
  Status s = DoTestSyncCall(p, "ThisMethodDoesNotExist");
  ASSERT_TRUE(s.IsRemoteError()) << "unexpected status: " << s.ToString();
  ASSERT_STR_CONTAINS(s.ToString(), 
                      "Service unavailable: service WrongServiceName "
                      "not registered on TestServer");
}            

/////////////////////
//

namespace {
int GetOpenFileLimit() {
  struct rlimit limit; 
  PCHECK(getrlimit(RLIMIT_NOFILE, &limit) == 0);
  return limit.rlim_cur;
} 
} 

TEST_P(TestRpc, TestHighFDs) {
  const int kNumFakeFiles = 3500;
  const int kMinUlimit = kNumFakeFiles + 100;
  if (GetOpenFileLimit() < kMinUlimit) { 
    LOG(INFO) << "Test skipped: must increase ulimit -n to at least " << kMinUlimit;
    return;
  } 
  
  vector<RandomAccessFile*> fake_files;
  ElementDeleter d(&fake_files);
  for (int i = 0; i < kNumFakeFiles; i++) {
    gscoped_ptr<RandomAccessFile> f; 
    CHECK_OK(Env::Default()->NewRandomAccessFile("/dev/zero", &f));
    fake_files.push_back(f.release());
  } 
  
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  ASSERT_OK(DoTestSyncCall(p, GenericCalculatorService::kAddMethodName));
} 

TEST_P(TestRpc, TestConnectionKeepalive) {
  n_server_reactor_threads_ = 1;
  keepalive_time_ms_ = 50;

  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);

  LOG(INFO) << "Connecting to " << server_addr.ToString();
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());

  ASSERT_OK(DoTestSyncCall(p, GenericCalculatorService::kAddMethodName));

  SleepFor(MonoDelta::FromMilliseconds(5));

  ReactorMetrics metrics;
  ASSERT_OK(server_messenger_->reactors_[0]->GetMetrics(&metrics));
  ASSERT_EQ(1, metrics.num_server_connections_) << "Server should have 1 server connection";
  ASSERT_EQ(0, metrics.num_client_connections_) << "Server should have 0 client connections";

  ASSERT_OK(client_messenger->reactors_[0]->GetMetrics(&metrics));
  ASSERT_EQ(0, metrics.num_server_connections_) << "Client should have 0 server connections";
  ASSERT_EQ(1, metrics.num_client_connections_) << "Client should have 1 client connections";

  SleepFor(MonoDelta::FromMilliseconds(100));

  ASSERT_OK(server_messenger_->reactors_[0]->GetMetrics(&metrics));
  ASSERT_EQ(0, metrics.num_server_connections_) << "Server should have 0 server connections";
  ASSERT_EQ(0, metrics.num_client_connections_) << "Server should have 0 client connections";

  ASSERT_OK(client_messenger->reactors_[0]->GetMetrics(&metrics));
  ASSERT_EQ(0, metrics.num_server_connections_) << "Client should have 0 server connections";
  ASSERT_EQ(0, metrics.num_client_connections_) << "Client should have 0 client connections";
}


TEST_P(TestRpc, TestCallLongerThanKeepalive) {
  keepalive_time_ms_ = 50;

  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  RpcController controller;
  SleepRequestPB req;
  req.set_sleep_micros(100 * 1000);
  req.set_deferred(true);
  SleepResponsePB resp;
  ASSERT_OK(p.SyncRequest(GenericCalculatorService::kSleepMethodName,
                          req, &resp, &controller));
}         

TEST_P(TestRpc, TestRpcSidecar) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, GetParam()));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  DoTestSidecar(p, 123, 456);
  
  DoTestSidecar(p, 3000 * 1024, 2000 * 1024);
} 

TEST_P(TestRpc, TestCallTimeout) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  ASSERT_NO_FATAL_FAILURE(DoTestExpectTimeout(p, MonoDelta::FromNanoseconds(1)));
  
  ASSERT_NO_FATAL_FAILURE(DoTestExpectTimeout(p, MonoDelta::FromMilliseconds(200)));
  
  ASSERT_NO_FATAL_FAILURE(DoTestExpectTimeout(p, MonoDelta::FromMilliseconds(1500)));
} 

TEST_P(TestRpc, TestCallTimeoutDoesntAffectNegotiation) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServer(&server_addr, enable_ssl);
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  FLAGS_rpc_negotiation_inject_delay_ms = 500;
  ASSERT_NO_FATAL_FAILURE(DoTestExpectTimeout(p, MonoDelta::FromMilliseconds(50)));
  ASSERT_OK(DoTestSyncCall(p, GenericCalculatorService::kAddMethodName));

  auto metric_map = server_messenger_->metric_entity()->UnsafeMetricsMapForTests();
  auto* metric = FindOrDie(metric_map, &METRIC_rpc_incoming_queue_time).get();
  ASSERT_EQ(1, down_cast<Histogram*>(metric)->TotalCount());
}

static void AcceptAndReadForever(Socket* listen_sock) {
  Socket server_sock;
  Sockaddr remote;
  CHECK_OK(listen_sock->Accept(&server_sock, &remote, 0));
  
  MonoTime deadline = MonoTime::Now() + MonoDelta::FromSeconds(10);
  
  size_t nread;
  uint8_t buf[1024];
  while (server_sock.BlockingRecv(buf, sizeof(buf), &nread, deadline).ok()) {
  }
} 

TEST_F(TestRpc, TestNegotiationTimeout) {
  Sockaddr server_addr;
  Socket listen_sock;
  ASSERT_OK(StartFakeServer(&listen_sock, &server_addr));
  
  scoped_refptr<Thread> acceptor_thread;
  ASSERT_OK(Thread::Create("test", "acceptor",
                                                             AcceptAndReadForever, &listen_sock,
                                                             &acceptor_thread));
                                  
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client"));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  ASSERT_NO_FATAL_FAILURE(DoTestExpectTimeout(p, MonoDelta::FromMilliseconds(100)));
  
  acceptor_thread->Join();
} 

TEST_F(TestRpc, TestServerShutsDown) {
  Sockaddr server_addr;
  Socket listen_sock;
  ASSERT_OK(StartFakeServer(&listen_sock, &server_addr));
  
  LOG(INFO) << "Connecting to " << server_addr.ToString();
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client"));
  Proxy p(client_messenger, server_addr, GenericCalculatorService::static_service_name());
  
  AddRequestPB req;
  req.set_x(rand());
  req.set_y(rand());
  AddResponsePB resp;
  
  vector<unique_ptr<RpcController>> controllers;
  
  int n_calls = 5;
  
  CountDownLatch latch(n_calls);
  for (int i = 0; i < n_calls; i++) {
    controllers.emplace_back(new RpcController());
    p.AsyncRequest(GenericCalculatorService::kAddMethodName, req, &resp, controllers.back().get(),
    //std::bind(&::ant::CountDownLatch::CountDown, std::ref(latch)));
    //std::bind(&CountDownLatch::CountDown, &latch);
        std::bind(static_cast<void (CountDownLatch::*)()>(&CountDownLatch::CountDown), std::ref(latch)));
  }                
  
  Socket server_sock;
  Sockaddr remote;
  ASSERT_OK(listen_sock.Accept(&server_sock, &remote, 0));
  
  for (const auto& controller : controllers) {
    ASSERT_FALSE(controller->finished());
  } 
  
  ASSERT_OK(listen_sock.Close());
  ASSERT_OK(server_sock.Close());

  latch.Wait();

  for (const auto& controller : controllers) {
    ASSERT_TRUE(controller->finished());
    Status s = controller->status();
    ASSERT_TRUE(s.IsNetworkError()) <<
      "Unexpected status: " << s.ToString();

    ASSERT_TRUE(s.posix_code() == EPIPE ||
                                s.posix_code() == ECONNRESET ||
                                s.posix_code() == ESHUTDOWN ||
                                s.posix_code() == ECONNREFUSED ||
                                s.posix_code() == EINVAL)
      << "Unexpected status: " << s.ToString();
  }
}

TEST_P(TestRpc, TestRpcHandlerLatencyMetric) {

  const uint64_t sleep_micros = 20 * 1000;

  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServerWithGeneratedCode(&server_addr, enable_ssl);

  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, CalculatorService::static_service_name());

  RpcController controller;
  SleepRequestPB req;
  req.set_sleep_micros(sleep_micros);
  req.set_deferred(true);
  SleepResponsePB resp;
  ASSERT_OK(p.SyncRequest("Sleep", req, &resp, &controller));

  const unordered_map<const MetricPrototype*, scoped_refptr<Metric> > metric_map =
    server_messenger_->metric_entity()->UnsafeMetricsMapForTests();

  scoped_refptr<Histogram> latency_histogram = down_cast<Histogram *>(
      FindOrDie(metric_map, &METRIC_handler_latency_ant_rpc_test_CalculatorService_Sleep).get());

  LOG(INFO) << "Sleep() min lat: " << latency_histogram->MinValueForTests();
  LOG(INFO) << "Sleep() mean lat: " << latency_histogram->MeanValueForTests();
  LOG(INFO) << "Sleep() max lat: " << latency_histogram->MaxValueForTests();
  LOG(INFO) << "Sleep() #calls: " << latency_histogram->TotalCount();

  ASSERT_EQ(1, latency_histogram->TotalCount());
  ASSERT_GE(latency_histogram->MaxValueForTests(), sleep_micros);
  ASSERT_TRUE(latency_histogram->MinValueForTests() == latency_histogram->MaxValueForTests());

  ASSERT_TRUE(FindOrDie(metric_map, &METRIC_rpc_incoming_queue_time));
}

static void DestroyMessengerCallback(shared_ptr<Messenger>* messenger,
                                     CountDownLatch* latch) {
  messenger->reset();
  latch->CountDown();
}

TEST_P(TestRpc, TestRpcCallbackDestroysMessenger) {
  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, GetParam()));
  Sockaddr bad_addr;
  CountDownLatch latch(1);

  AddRequestPB req;
  req.set_x(rand());
  req.set_y(rand());
  AddResponsePB resp;
  RpcController controller;
  controller.set_timeout(MonoDelta::FromMilliseconds(1));
  {
    Proxy p(client_messenger, bad_addr, "xxx");
    p.AsyncRequest("my-fake-method", req, &resp, &controller,
                   std::bind(&DestroyMessengerCallback, &client_messenger, &latch));
  }
  latch.Wait();
}

//////////////

TEST_P(TestRpc, TestRpcContextClientDeadline) {
  const uint64_t sleep_micros = 20 * 1000;

  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServerWithGeneratedCode(&server_addr, enable_ssl);

  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, CalculatorService::static_service_name());
  
  SleepRequestPB req;
  req.set_sleep_micros(sleep_micros);
  req.set_client_timeout_defined(true);
  SleepResponsePB resp;
  RpcController controller;
  Status s = p.SyncRequest("Sleep", req, &resp, &controller);
  ASSERT_TRUE(s.IsRemoteError());
  ASSERT_STR_CONTAINS(s.ToString(), "Missing required timeout");
  
  controller.Reset();
  controller.set_timeout(MonoDelta::FromMilliseconds(1000));
  ASSERT_OK(p.SyncRequest("Sleep", req, &resp, &controller));
} 

TEST_P(TestRpc, TestApplicationFeatureFlag) {
  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServerWithGeneratedCode(&server_addr, enable_ssl);

  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, CalculatorService::static_service_name());

  { // Supported flag
       AddRequestPB req;
       req.set_x(1);
       req.set_y(2);
       AddResponsePB resp;
       RpcController controller;
       controller.RequireServerFeature(FeatureFlags::FOO);
       Status s = p.SyncRequest("Add", req, &resp, &controller);
       SCOPED_TRACE(strings::Substitute("supported response: $0", s.ToString()));
       ASSERT_TRUE(s.ok());
       ASSERT_EQ(resp.result(), 3);
     }

  { // Unsupported flag
       AddRequestPB req;
       req.set_x(1);
       req.set_y(2);
       AddResponsePB resp;
       RpcController controller;
       controller.RequireServerFeature(FeatureFlags::FOO);
       controller.RequireServerFeature(99);
       Status s = p.SyncRequest("Add", req, &resp, &controller);
       SCOPED_TRACE(strings::Substitute("unsupported response: $0", s.ToString()));
       ASSERT_TRUE(s.IsRemoteError());
     }
}

TEST_P(TestRpc, TestApplicationFeatureFlagUnsupportedServer) {
  auto savedFlags = kSupportedServerRpcFeatureFlags;
  auto cleanup = MakeScopedCleanup([&] () { kSupportedServerRpcFeatureFlags = savedFlags; });
  kSupportedServerRpcFeatureFlags = {};

  Sockaddr server_addr;
  bool enable_ssl = GetParam();
  StartTestServerWithGeneratedCode(&server_addr, enable_ssl);

  shared_ptr<Messenger> client_messenger(CreateMessenger("Client", 1, enable_ssl));
  Proxy p(client_messenger, server_addr, CalculatorService::static_service_name());

  { // Required flag
       AddRequestPB req;
       req.set_x(1);
       req.set_y(2);
       AddResponsePB resp;
       RpcController controller;
       controller.RequireServerFeature(FeatureFlags::FOO);
       Status s = p.SyncRequest("Add", req, &resp, &controller);
       SCOPED_TRACE(strings::Substitute("supported response: $0", s.ToString()));
       ASSERT_TRUE(s.IsNotSupported());
     }

  { // No required flag
       AddRequestPB req;
       req.set_x(1);
       req.set_y(2);
       AddResponsePB resp;
       RpcController controller;
       Status s = p.SyncRequest("Add", req, &resp, &controller);
       SCOPED_TRACE(strings::Substitute("supported response: $0", s.ToString()));
       ASSERT_TRUE(s.ok());
     }
}


} // namespace rpc
} // namespace ant
