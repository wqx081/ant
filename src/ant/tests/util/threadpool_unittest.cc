#include "ant/util/threadpool.h"

#include <functional>
#include <memory>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "ant/base/atomicops.h"
#include "ant/base/bind.h"
#include "ant/util/countdown_latch.h"
#include "ant/util/metrics.h"
#include "ant/util/promise.h"
#include "ant/util/test_macros.h"
#include "ant/util/trace.h"
#include "ant/util/yield.h"

namespace ant {

namespace {

static Status BuildMinMaxTestPool(int min_threads,
                                  int max_threads,
                                  gscoped_ptr<ThreadPool>* pool) {
  return ThreadPoolBuilder("test").set_min_threads(min_threads)
                                  .set_max_threads(max_threads)
                                  .Build(pool);
}

} // namespace

TEST(ThreadPool, Base) {
  gscoped_ptr<ThreadPool> thread_pool;
  ASSERT_OK(BuildMinMaxTestPool(8, 8, &thread_pool));
  thread_pool->Shutdown();
}


static void SimpleTaskMethod(int n, base::subtle::Atomic32* counter) {
  while (n--) {
    base::subtle::NoBarrier_AtomicIncrement(counter, 1);
    ant::yield(n);
  }
}

class SimpleTask : public Runnable {
 public:
  SimpleTask(int n, base::subtle::Atomic32* counter)
    : n_(n), counter_(counter) {}

  void Run() override {
    SimpleTaskMethod(n_, counter_);
  }

 private:
  int n_;
  base::subtle::Atomic32* counter_;
};

TEST(ThreadPool, SimpleTasks) {
  gscoped_ptr<ThreadPool> thread_pool;
  ASSERT_OK(BuildMinMaxTestPool(4, 4, &thread_pool));

  base::subtle::Atomic32 counter(0);
  std::shared_ptr<Runnable> task(new SimpleTask(15, &counter));

  ASSERT_OK(thread_pool->SubmitFunc(std::bind(&SimpleTaskMethod, 10, &counter)));
  ASSERT_OK(thread_pool->Submit(task));
  ASSERT_OK(thread_pool->SubmitFunc(std::bind(&SimpleTaskMethod, 20, &counter)));
  ASSERT_OK(thread_pool->Submit(task));
  ASSERT_OK(thread_pool->SubmitClosure(base::Bind(&SimpleTaskMethod, 123, &counter)));
  thread_pool->Wait();
  ASSERT_EQ(10+15+20+15+123, base::subtle::NoBarrier_Load(&counter));
  thread_pool->Shutdown();
}

//////// 
static void IssueTraceStatement() {
  TRACE("hello from task");
}

TEST(ThreadPool, TestTracePropagation) {
  gscoped_ptr<ThreadPool> thread_pool;
  ASSERT_OK(BuildMinMaxTestPool(1, 1, &thread_pool));
  
  scoped_refptr<Trace> t(new Trace);
  {
    ADOPT_TRACE(t.get());
    ASSERT_OK(thread_pool->SubmitFunc(&IssueTraceStatement));
  }
  thread_pool->Wait();
  ASSERT_STR_CONTAINS(t->DumpToString(), "hello from task");
}
  
TEST(ThreadPool, TestSubmitAfterShutdown) {
  gscoped_ptr<ThreadPool> thread_pool;
  ASSERT_OK(BuildMinMaxTestPool(1, 1, &thread_pool));
  thread_pool->Shutdown();
  Status s = thread_pool->SubmitFunc(&IssueTraceStatement);
  ASSERT_EQ("Service unavailable: The pool has been shut down.", s.ToString());
}


} // namespace ant
