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
  SimpleThreadPool(int thread_count = 0) : done_(false), joiner_(threads_) {
    if (!thread_count) {
      thread_count = std::thread::hardware_concurrency();
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

  template<typename FunctionType>
  void SubmitWork(FunctionType f) {   // submit work to be done
    work_queue_.push(std::function<void()>(f));
  }

 private:
  void WorkerThread() {
    while (!done_) {
      std::function<void()> task;
      if (work_queue_.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

  std::atomic_bool done_;   // boolean to stop the running thread
  threadsafe_queue<std::function<void()>> work_queue_;    // next work item on the queue
  std::vector<std::thread> threads_;    // store all the threads in the pool
  JoinThreads joiner_;    // clean up and join all the threads in the pool
};
}   // namespace thread_pool
#endif  // SRC_SIMPLE_TPOOL_H_
