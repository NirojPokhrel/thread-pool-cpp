#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>

#include "src/work_stealing_queue.h"

TEST (WorkStealingQueue, PushAndPop) {
  thread_pool::WorkStealingQueue queue;
  auto adder = [](int a, int b) {return a + b;};
  std::packaged_task<int()> pkg_task(std::bind(adder, 10, 20));
  auto fut = pkg_task.get_future();
  queue.Push(std::move(pkg_task));
  thread_pool::FunctionWrapper task;
  EXPECT_TRUE(queue.TryPop(task));
  task();
  EXPECT_EQ(30, fut.get());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}