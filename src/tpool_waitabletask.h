#ifndef SRC_TPOOL_WAITABLETASK_H_
#define SRC_TPOOL_WAITABLETASK_H_
#include <atomic>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "src/function_wrapper.h"
#include "src/join_threads.h"
#include "src/threadsafe_queue.h"

namespace thread_pool {
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
  std::future<typename std::result_of<FunctionType()>::type> Submit(FunctionType f) {
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
