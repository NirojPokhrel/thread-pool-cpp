#ifndef SRC_TPOOL_WAITABLETASK_H_
#define SRC_TPOOL_WAITABLETASK_H_
#include <atomic>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "include/function_wrapper.h"
#include "include/join_threads.h"
#include "include/threadsafe_queue.h"

namespace thread_pool {
class TpoolWaitableTask {
 public:
  // construct a thread pool
  // @thread_count - provide a number of threads to instantiate in thread pool otherwise it will default to
  //                 number of hardware threads
  TpoolWaitableTask(int thread_count = 0) : done_(false), joiner_(threads_) {
    if (!thread_count) {
      thread_count = std::thread::hardware_concurrency();
      if (!thread_count) {
        thread_count = 2;
      }
    }
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::thread(&TpoolWaitableTask::WorkerThread, this));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  // destroy the thread pool
  ~TpoolWaitableTask() {
    done_ = true;
  }

  // submit work to be done to the thread pool
  // @f work to be done
  // returns future which stores return value
  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> Submit(FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());
    work_queue_.Push(std::move(task));
    return res;
  }

 private:
  void WorkerThread() {
    while (!done_) {
      FunctionWrapper task;
      if (work_queue_.TryPop(task)) {
        task();
      } else {
        std::this_thread::yield;
      }
    }
  }
  std::atomic_bool done_;   // boolean to stop the running thread
  ThreadsafeQueue<FunctionWrapper> work_queue_;  // next work item on the queue
  std::vector<std::thread> threads_;    // store all the threads in the pool
  JoinThreads joiner_;    // clean up and join all the threads in the pool
};
}   // namespace thread_pool
#endif  // SRC_TPOOL_WAITABLETASK_H_
