#ifndef SRC_FUNCTION_WRAPPER_H_
#define SRC_FUNCTION_WRAPPER_H_
#include <memory>
namespace thread_pool {
// std::packaged_task<> instances are not copyable, just movable, std::function<> for the queue entries
// can no longer be used for queue entries because std::function<> requires that the stored function objects
// are copy-constructible. Thus, FunctionWrapper is used
class FunctionWrapper {
 public:
  template<typename F>
  FunctionWrapper(F&& f) : impl_(new ImplType<F>(std::move(f))) {}

  FunctionWrapper() = default;

  void operator() () { impl_->Call(); }

  FunctionWrapper(FunctionWrapper&& other) : impl_(std::move(other.impl_)) {}

  FunctionWrapper& operator=(FunctionWrapper&& other) {
    impl_ = std::move(other.impl_);
    return *this;
  }
  FunctionWrapper(const FunctionWrapper&) = delete;
  FunctionWrapper(FunctionWrapper&) = delete;
  FunctionWrapper& operator=(const FunctionWrapper&) = delete;

 private:
  struct ImplBase {
    virtual void Call() = 0;
    virtual ~ImplBase() {}
  };
  template<typename F>
  struct ImplType : ImplBase {
    F f_;
    ImplType(F&& f) : f_(std::move(f)) {}
    void Call() { f_(); }
  };
  std::unique_ptr<ImplBase> impl_;
};
}  // namespace thread_pool
#endif  // SRC_FUNCTION_WRAPPER_H_