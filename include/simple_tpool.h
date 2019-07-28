#ifndef SRC_SIMPLE_TPOOL_H_
#define SRC_SIMPLE_TPOOL_H_
#include <atomic>
#include <queue>
#include <thread>
#include <vector>

#include "join_threads.h"
#include "threadsafe_queue.h"
namespace thread_pool {
class SimpleThreadPool {
 public:
  // construct a simple thread pool
  // @thread_count - provide a number of threads to instantiate in thread pool otherwise it will default to
  //                 number of hardware threads
  explicit SimpleThreadPool(int thread_count = 0) : done_(false), joiner_(threads_) {
    if (!thread_count) {
      thread_count = std::thread::hardware_concurrency();
      if (!thread_count) {
        thread_count = 2;
      }
    }
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::thread(&SimpleThreadPool::WorkerThread, this));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  ~SimpleThreadPool() {
    done_ = true;
  }

  // submit the work to be done by threadpool
  // @f work to be done
  template<typename FunctionType>
  void SubmitWork(FunctionType f) {   // submit work to be done
    work_queue_.Push(std::function<void()>(f));
  }

 private:
  void WorkerThread() {
    while (!done_) {
      std::function<void()> task;
      if (work_queue_.TryPop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

  std::atomic_bool done_;   // boolean to stop the running thread
  ThreadsafeQueue<std::function<void()>> work_queue_;    // next work item on the queue
  std::vector<std::thread> threads_;    // store all the threads in the pool
  JoinThreads joiner_;    // clean up and join all the threads in the pool
};
}   // namespace thread_pool
#endif  // SRC_SIMPLE_TPOOL_H_
