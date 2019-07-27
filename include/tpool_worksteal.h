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
  WorkStealTpool() : done_(false), joiner_(threads_) {
    unsigned const thread_count = std::thread::hardware_concurrency();
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

  ~WorkStealTpool() {
    done_ = true;
  }

  template<typename FunctionType>
  std::future<typename std::result_of<FunctionType()>::type> SubmitWork (FunctionType f) {
    typedef typename std::result_of<FunctionType()>::type result_type;
    std::packaged_task<result_type()> task(f);
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->Push(std::move(task));
    } else {
      pool_work_queue_.push(std::move(task));
    }
    return res;
  }

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
    return pool_work_queue_.try_pop(task);
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

  std::atomic<bool> done_;
  threadsafe_queue<FunctionWrapper> pool_work_queue_;
  std::vector<std::unique_ptr<WorkStealingQueue>> queues_;
  std::vector<std::thread> threads_;
  JoinThreads joiner_;
  static thread_local WorkStealingQueue* local_work_queue_;
  static thread_local unsigned local_index_;
};
thread_local WorkStealingQueue* WorkStealTpool::local_work_queue_;
thread_local unsigned WorkStealTpool::local_index_;
}
#endif  // SRC_TPOOL_WORKSTEAL_H_