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
  // construct a thread pool with local work queue
  // @thread_count - provide a number of threads to instantiate in thread pool otherwise it will default to
  //                 number of hardware threads
  explicit ThreadPoolLocalWorkQueue(int thread_count = 0) : done_(false), joiner_(threads_) {
    if (!thread_count) {
      thread_count = std::thread::hardware_concurrency();
      if (!thread_count) {
        thread_count = 2;
      }
    }
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

  // submit the work to be done by threadpool
  // @f work to be done
  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> Submit(FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(f);
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->push(std::move(task));
    } else {
      ts_queue_.Push(std::move(task));
    }
    return res;
  }

  // run pending task if available by checking local work queue, thread pool queue
  //                  if not available yield
  void RunPendingTask() {
    FunctionWrapper task;
    if (local_work_queue_ && !local_work_queue_->empty()) {
      task = std::move(local_work_queue_->front());
      local_work_queue_->pop();
      task();
    } else if (ts_queue_.TryPop(task)) {
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
  std::atomic<bool> done_;  // flag set to stop thread pool
  std::vector<std::thread> threads_;  // threads running in this thread pool
  JoinThreads joiner_;  // Raii class for joining all the threads
  ThreadsafeQueue<FunctionWrapper> ts_queue_;  // thread safe queue for the thread pool work storage
  typedef std::queue<FunctionWrapper> local_queue_type;
  static thread_local std::unique_ptr<local_queue_type> local_work_queue_;  // local work queue for each thread in thread pool

};
}  // namespace thread_pool
// following can be declared in .cpp files when implementing it
thread_local std::unique_ptr<std::queue<thread_pool::FunctionWrapper>> thread_pool::ThreadPoolLocalWorkQueue::local_work_queue_;
#endif  // SRC_TPOOL_LOCALQUEUE_H_
