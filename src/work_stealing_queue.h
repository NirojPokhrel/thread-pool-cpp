#ifndef SRC_WORK_STEALING_QUEUE_H_
#define SRC_WORK_STEALING_QUEUE_H_
#include <deque>
#include <mutex>
#include "src/function_wrapper.h"
namespace thread_pool {
class WorkStealingQueue {
 public:
  WorkStealingQueue() {}
  WorkStealingQueue(const WorkStealingQueue& other) = delete;
  WorkStealingQueue& operator=(const WorkStealingQueue& other) = delete;

  void Push(FunctionWrapper data) {
    std::lock_guard<std::mutex> lock(mtx_);
    queue_.push_front(std::move(data));
  }

  bool Empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
  }

  bool TryPop(FunctionWrapper& res) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (queue_.empty()) {
      return false;
    }
    res = std::move(queue_.front());
    queue_.pop_back();
    return true;
  }

  bool TrySteal(FunctionWrapper& res) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (queue_.empty()) {
      return false;
    }
    res = std::move(queue_.back());
    queue_.pop_back();
    return true;
  }

 private:
  std::deque<FunctionWrapper> queue_;
  mutable std::mutex mtx_;
};
}  // namespace thread_pool
#endif  // SRC_WORK_STEALING_QUEUE_H_
