#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "src/simple_tpool.h"

TEST (SimpleThreadPool, SubmitDifferentTask) {
  int a = 5, b = 10, c = 15, d = 20;
  auto Addition = [](int x, int y, int& res) -> void {
    for (int i=0; i<y; i++) {
      res += x;
      std::this_thread::yield;  // let another thread run after each iteration
    }
  };
  int e = 0, f = 0;
  thread_pool::ThreadPool tpool;
  tpool.SubmitWork(std::bind(Addition, a, b, std::ref(e)));   // std::ref is required to let bind operation know it is references
  tpool.SubmitWork(std::bind(Addition, c, d, std::ref(f)));   // otherwise, bind operations copies the variable and this copy's reference is passed to the lambda
  std::this_thread::sleep_for(std::chrono::milliseconds(100));  // let child threads complete
  EXPECT_EQ(e, 50);
  EXPECT_EQ(f, 300);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}