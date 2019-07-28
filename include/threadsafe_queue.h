#ifndef SRC_THREASAFE_QUEUE_H_
#define SRC_THREASAFE_QUEUE_H_
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace thread_pool {
template<typename T>
class ThreadsafeQueue {
 public:
  ThreadsafeQueue() {}
  ThreadsafeQueue(const ThreadsafeQueue& queue) {
    std::lock_guard<std::mutex> guard(mtx_);
    this->std_queue_ = queue.std_queue_;
  }
  ThreadsafeQueue& operator=(const ThreadsafeQueue&) = delete;    // delete the assignment operation

  void Push(T new_value) {
    std::lock_guard<std::mutex> guard(mtx_);
    std_queue_.push(std::move(new_value));
    cond_var_.notify_one();
  }

  void WaitAndPop(T& value) {
    std::unique_lock<std::mutex>  u_lk(mtx_);
    cond_var_.wait(u_lk, [this](){return !std_queue_.empty();});
    value = std_queue_.front();
    std_queue_.pop();
  }

  std::shared_ptr<T> WaitAndPop() {
    std::unique_lock<std::mutex> u_lk(mtx_);
    cond_var_.wait(u_lk, [this](){return !std_queue_.empty();});
    std::shared_ptr<T> res(std::make_shared<T>(std_queue_.front()));
    std_queue_.pop();
    return res;
  }

  bool TryPop(T& value) {
    std::lock_guard<std::mutex> guard(mtx_);
    if (std_queue_.empty()) {
      return false;
    }
    value = std::move(std_queue_.front());
    std_queue_.pop();
    return true;
  }

  std::shared_ptr<T> TryPop() {
    std::lock_guard<std::mutex> guard(mtx_);
    if (std_queue_.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(std::make_shared<T>(std_queue_.front()));
    std_queue_.pop();
    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> guard(mtx_);  // mtx_ changed in const functions so needs to be defined mutable
    return std_queue_.empty();
  }

 private:
  mutable std::mutex  mtx_;             // mutex to protect the queue
  std::queue<T> std_queue_;             // standard queue
  std::condition_variable cond_var_;    // condition variable for synhronization
};
}   // namespace thread_pool
#endif  // SRC_THREASAFE_QUEUE_H_