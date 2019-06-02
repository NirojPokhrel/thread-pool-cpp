#include SRC_TPOOL_WAITABLETASK_H_
#define SRC_TPOOL_WAITABLETASK_H_
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "src/threadsafe_queue.h"
#include "src/join_threads.h"
namespace thread_pool {
// std::packaged_task<> instances are not copyable, just movable, std::function<> for the queue entries
// can no longer be used for queue entries because std::function<> requires that the stored function objects
// are copy-constructible. Thus, FunctionWrapper is used
class FunctionWrapper {
 public:
  template<typename F>
  FunctionWrapper(F&& f) :  impl_(new ImplType<F>(std::move(f))) {}

  void operator() () { impl_->Call(); }

  FunctionWrapper& operator=(FunctionWrapper&& other) {
    impl_ = std::move(other.impl_);
    return *this;
  }
  FunctionWrapper(const FunctionWrapper&) = delete;
  FunctionWrapper(FunctionWrapper&) = delete;
  FunctionWrapper& operator=(const FunctionWrapper&) = delete;

 private:
  struct ImplBase {
    virtual void Call() = 0;
    virtual ~ImplBase() {}
  };
  template<typename F>
  struct ImplType : ImplBase {
    F f_;
    ImplType(F&& f) : f_(std::move(f)) {}
    void Call() { f_(); }
  };
  std::unique_ptr<ImplBase> impl_;
};

class TpoolWaitableTask {
 public:
  TpoolWaitableTask() : done_(false), joiner_(threads_) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::thread(&TpoolWaitableTask::WorkerThread, this));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  ~TpoolWaitableTask() {
    done_ = true;
  }

  template<typename FunctionType>
  std::future<typename std::result_of<FuntionType()>::type> submit(FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());
    work_queue_.push(std::move(task));
    return res;
  }

 private:
  void WorkerThread() {
    while (!done_) {
      FunctionWrapper task;
      if (work_queue_.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield;
      }
    }
  }
  std::atomic_bool done_;   // boolean to stop the running thread
  threadsafe_queue<FunctionWrapper> work_queue_;  // next work item on the queue
  std::vector<std::thread> threads_;    // store all the threads in the pool
  JoinThreads joiner_;    // clean up and join all the threads in the pool
};
}   // namespace thread_pool
#endif  // SRC_TPOOL_WAITABLETASK_H_
