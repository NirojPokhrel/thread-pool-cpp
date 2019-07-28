#ifndef SRC_TPOOL_WORKSTEAL_H_
#define SRC_TPOOL_WORKSTEAL_H_
#include <numeric>
#include <thread>
#include "function_wrapper.h"
#include "join_threads.h"
#include "threadsafe_queue.h"
#include "work_stealing_queue.h"
namespace thread_pool {
// Each threads in worksteal thread pool has it's own queue. However, threads which are currently not busy can steal
// work from the threads that are busy.
class WorkStealTpool {
 public:
  // construct a thread pool with local queue
  // @thread_count - provide a number of threads to instantiate in thread pool otherwise it will default to
  //                 number of hardware threads
  explicit WorkStealTpool(int thread_count = 0) : done_(false), joiner_(threads_) {
    if (!thread_count) {
      thread_count = std::thread::hardware_concurrency();
      if (!thread_count) {
        thread_count = 2;
      }
    }
    try {
      for (unsigned i=0; i<thread_count; ++i) {
        queues_.push_back(std::unique_ptr<WorkStealingQueue>(new WorkStealingQueue()));
      }
      for (unsigned i=0; i<thread_count; ++i) {
         threads_.push_back(std::thread(&WorkStealTpool::WorkerThread, this, i));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  // destroy the thread pool
  ~WorkStealTpool() {
    done_ = true;
  }

  // submit work to be done to the thread pool
  // @f work to be done
  // returns future which stores return value
  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> SubmitWork (FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(f);
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->Push(std::move(task));
    } else {
      pool_work_queue_.Push(std::move(task));
    }
    return res;
  }

  // run pending task if available by checking local work queue, thread pool queue or stealing from other queue
  //                  if not available yield
  void RunPendingTask() {
    FunctionWrapper task;
    if (PopTaskFromLocalQueue(task) || PopTaskFromPoolQueue(task) || PopTaskFromOtherThreadQueue(task)) {
      task();
    } else {
      std::this_thread::yield();
    }
  }

 private:
  void WorkerThread(unsigned index) {
    local_index_ = index;
    local_work_queue_ = queues_[local_index_].get();
    while (!done_) {
      RunPendingTask();
    }
  }

  bool PopTaskFromLocalQueue(FunctionWrapper& task) {
    return local_work_queue_ && local_work_queue_->TryPop(task);
  }

  bool PopTaskFromPoolQueue(FunctionWrapper& task) {
    return pool_work_queue_.TryPop(task);
  }

  bool PopTaskFromOtherThreadQueue(FunctionWrapper& task) {
    for (unsigned i=0; i<queues_.size(); ++i) {
      unsigned const index = (local_index_ + i + 1) % queues_.size();
      if (queues_[index]->TrySteal(task)) {
        return true;
      }
    }
    return false;
  }

  std::atomic<bool> done_;  // flag set to stop thread pool
  ThreadsafeQueue<FunctionWrapper> pool_work_queue_;  // thread safe queue for the thread pool work storage
  std::vector<std::unique_ptr<WorkStealingQueue>> queues_;  // collection of all local work queues which allows work stealing
  std::vector<std::thread> threads_;  // threads running in this thread pool
  JoinThreads joiner_;  // Raii class for joining all the threads
  static thread_local WorkStealingQueue* local_work_queue_;  // pointer to local work queue for each thread in thread pool
  static thread_local unsigned local_index_;  // index of work queue
};
// todo (niroj) : following shoule be moved in cpp file
thread_local WorkStealingQueue* WorkStealTpool::local_work_queue_;
thread_local unsigned WorkStealTpool::local_index_;
}
#endif  // SRC_TPOOL_WORKSTEAL_H_