#ifndef SRC_TPOOL_LOCALQUEUE_H_
#define SRC_TPOOL_LOCALQUEUE_H_
#include <atomic>
#include <future>
#include <thread>
#include <vector>

#include "include/function_wrapper.h"
#include "include/join_threads.h"
#include "include/threadsafe_queue.h"
namespace thread_pool {
class ThreadPoolLocalWorkQueue {
 public:
  ThreadPoolLocalWorkQueue() : done_(false), joiner_(threads_) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::thread(&ThreadPoolLocalWorkQueue::WorkerThread, this));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  ~ThreadPoolLocalWorkQueue() {
    done_ = true;
  }

  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> Submit(FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(f);
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->push(std::move(task));
    } else {
      ts_queue_.push(std::move(task));
    }
    return res;
  }

  void RunPendingTask() {
    FunctionWrapper task;
    if (local_work_queue_ && !local_work_queue_->empty()) {
      task = std::move(local_work_queue_->front());
      local_work_queue_->pop();
      task();
    } else if (ts_queue_.try_pop(task)) {
      task();
    } else {
      std::this_thread::yield();
    }
  }

 private:
  void WorkerThread() {
    local_work_queue_.reset(new local_queue_type);
    while (!done_) {
      RunPendingTask();
    }
  }
  std::atomic<bool> done_;
  std::vector<std::thread> threads_;
  JoinThreads joiner_;
  threadsafe_queue<FunctionWrapper> ts_queue_;
  typedef std::queue<FunctionWrapper> local_queue_type;
  static thread_local std::unique_ptr<local_queue_type> local_work_queue_;

};
}  // namespace thread_pool
thread_local std::unique_ptr<std::queue<thread_pool::FunctionWrapper>> thread_pool::ThreadPoolLocalWorkQueue::local_work_queue_;
#endif  // SRC_TPOOL_LOCALQUEUE_H_
