#ifndef SRC_SIMPLE_TPOOL_H_
#define SRC_SIMPLE_TPOOL_H_
#include <atomic>
#include <queue>
#include <thread>
#include <vector>

#include "join_threads.h"
#include "threadsafe_queue.h"
namespace thread_pool {
class ThreadPool {
 public:
  ThreadPool() : done_(false), joiner_(threads_) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::thread(&ThreadPool::WorkerThread, this));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  ~ThreadPool() {
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
}
#endif  // SRC_SIMPLE_TPOOL_H_
