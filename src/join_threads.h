#ifndef SRC_JOIN_THREADS_H_
#define SRC_JOIN_THREADS_H_
#include <thread>
#include <vector>
namespace thread_pool {
// RAII class for joining all the threads
class JoinThreads {
 public:
  explicit JoinThreads(std::vector<std::thread>& threads) : threads_(threads) {}
  ~JoinThreads() {
    for (size_t i=0; i<threads_.size(); ++i) {
      if (threads_[i].joinable()) {
        threads_[i].join():
      }
    }
  }

 private:
  std::vector<std::thread>& threads_;
};
}
#endif  // SRC_JOIN_THREADS_H_