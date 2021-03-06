#include "ant/rpc/service_pool.h"

#include <glog/logging.h>
#include <memory>
#include <string>
#include <vector>

#include "ant/base/gscoped_ptr.h"
#include "ant/base/ref_counted.h"
#include "ant/rpc/inbound_call.h"
#include "ant/rpc/messenger.h"
#include "ant/rpc/service_if.h"
#include "ant/rpc/service_queue.h"
#include "ant/base/strings/substitute.h"
//#include "ant/util/logging.h"
#include "ant/util/metrics.h"
#include "ant/util/status.h"
#include "ant/util/thread.h"
#include "ant/util/trace.h"

using std::shared_ptr;
using strings::Substitute;

METRIC_DEFINE_histogram(server, rpc_incoming_queue_time,
                        "RPC Queue Time",
                        ant::MetricUnit::kMicroseconds,
                        "Number of microseconds incoming RPC requests spend in the worker queue",
                        60000000LU, 3);

METRIC_DEFINE_counter(server, rpcs_timed_out_in_queue,
                      "RPC Queue Timeouts",
                      ant::MetricUnit::kRequests,
                      "Number of RPCs whose timeout elapsed while waiting "
                      "in the service queue, and thus were not processed.");

METRIC_DEFINE_counter(server, rpcs_queue_overflow,
                      "RPC Queue Overflows",
                      ant::MetricUnit::kRequests,
                      "Number of RPCs dropped because the service queue "
                      "was full.");

namespace ant {
namespace rpc {

ServicePool::ServicePool(gscoped_ptr<ServiceIf> service,
                         const scoped_refptr<MetricEntity>& entity,
                         size_t service_queue_length)
  : service_(std::move(service)),
    service_queue_(service_queue_length),
    incoming_queue_time_(METRIC_rpc_incoming_queue_time.Instantiate(entity)),
    rpcs_timed_out_in_queue_(METRIC_rpcs_timed_out_in_queue.Instantiate(entity)),
    rpcs_queue_overflow_(METRIC_rpcs_queue_overflow.Instantiate(entity)),
    closing_(false) {
}

ServicePool::~ServicePool() {
  Shutdown();
}

Status ServicePool::Init(int num_threads) {
  for (int i = 0; i < num_threads; i++) {
    scoped_refptr<ant::Thread> new_thread;
    CHECK_OK(ant::Thread::Create("service pool", "rpc worker",
        &ServicePool::RunThread, this, &new_thread));
    threads_.push_back(new_thread);
  }
  return Status::OK();
}

void ServicePool::Shutdown() {
  service_queue_.Shutdown();

  MutexLock lock(shutdown_lock_);
  if (closing_) return;
  closing_ = true;
  // TODO: Use a proper thread pool implementation.
  for (scoped_refptr<ant::Thread>& thread : threads_) {
    CHECK_OK(ThreadJoiner(thread.get()).Join());
  }

  // Now we must drain the service queue.
  Status status = Status::ServiceUnavailable("Service is shutting down");
  std::unique_ptr<InboundCall> incoming;
  while (service_queue_.BlockingGet(&incoming)) {
    incoming.release()->RespondFailure(ErrorStatusPB::FATAL_SERVER_SHUTTING_DOWN, status);
  }

  service_->Shutdown();
}

void ServicePool::RejectTooBusy(InboundCall* c) {
  string err_msg =
      Substitute("$0 request on $1 from $2 dropped due to backpressure. "
                 "The service queue is full; it has $3 items.",
                 c->remote_method().method_name(),
                 service_->service_name(),
                 c->remote_address().ToString(),
                 service_queue_.max_size());
  rpcs_queue_overflow_->Increment();
////  KLOG_EVERY_N_SECS(WARNING, 1) << err_msg;
  c->RespondFailure(ErrorStatusPB::ERROR_SERVER_TOO_BUSY,
                    Status::ServiceUnavailable(err_msg));
  DLOG(INFO) << err_msg << " Contents of service queue:\n"
             << service_queue_.ToString();
}

RpcMethodInfo* ServicePool::LookupMethod(const RemoteMethod& method) {
  return service_->LookupMethod(method);
}

Status ServicePool::QueueInboundCall(gscoped_ptr<InboundCall> call) {
  InboundCall* c = call.release();

  vector<uint32_t> unsupported_features;
  for (uint32_t feature : c->GetRequiredFeatures()) {
    if (!service_->SupportsFeature(feature)) {
      unsupported_features.push_back(feature);
    }
  }

  if (!unsupported_features.empty()) {
    c->RespondUnsupportedFeature(unsupported_features);
    return Status::NotSupported("call requires unsupported application feature flags");
  }

  TRACE_TO(c->trace(), "Inserting onto call queue");

  // Queue message on service queue
  boost::optional<InboundCall*> evicted;
  auto queue_status = service_queue_.Put(c, &evicted);
  if (queue_status == QUEUE_FULL) {
    RejectTooBusy(c);
    return Status::OK();
  }

  if (PREDICT_FALSE(evicted != boost::none)) {
    RejectTooBusy(*evicted);
  }

  if (PREDICT_TRUE(queue_status == QUEUE_SUCCESS)) {
    // NB: do not do anything with 'c' after it is successfully queued --
    // a service thread may have already dequeued it, processed it, and
    // responded by this point, in which case the pointer would be invalid.
    return Status::OK();
  }

  Status status = Status::OK();
  if (queue_status == QUEUE_SHUTDOWN) {
    status = Status::ServiceUnavailable("Service is shutting down");
    c->RespondFailure(ErrorStatusPB::FATAL_SERVER_SHUTTING_DOWN, status);
  } else {
    status = Status::RuntimeError(Substitute("Unknown error from BlockingQueue: $0", queue_status));
    c->RespondFailure(ErrorStatusPB::FATAL_UNKNOWN, status);
  }
  return status;
}

void ServicePool::RunThread() {
  while (true) {
    std::unique_ptr<InboundCall> incoming;
    if (!service_queue_.BlockingGet(&incoming)) {
      VLOG(1) << "ServicePool: messenger shutting down.";
      return;
    }

    incoming->RecordHandlingStarted(incoming_queue_time_);
    ADOPT_TRACE(incoming->trace());

    if (PREDICT_FALSE(incoming->ClientTimedOut())) {
      TRACE_TO(incoming->trace(), "Skipping call since client already timed out");
      rpcs_timed_out_in_queue_->Increment();

      // Respond as a failure, even though the client will probably ignore
      // the response anyway.
      incoming->RespondFailure(
        ErrorStatusPB::ERROR_SERVER_TOO_BUSY,
        Status::TimedOut("Call waited in the queue past client deadline"));

      // Must release since RespondFailure above ends up taking ownership
      // of the object.
      ignore_result(incoming.release());
      continue;
    }

    TRACE_TO(incoming->trace(), "Handling call");

    // Release the InboundCall pointer -- when the call is responded to,
    // it will get deleted at that point.
    service_->Handle(incoming.release());
  }
}

const string ServicePool::service_name() const {
  return service_->service_name();
}

} // namespace rpc
} // namespace kudu
