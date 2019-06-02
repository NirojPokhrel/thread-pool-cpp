
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <future>
#include <vector>
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

TEST (SimpleThreadPool, ParallelAccumulateTenElem) {
  std::vector<int> input_vec;
  int num_of_elem = 100;    // Set it multiple of 10
  input_vec.reserve(num_of_elem);
  for (int i=0; i<num_of_elem; i++) {
    input_vec.emplace_back(i);
  }
  auto Addition = [] (const std::vector<int>& input_vec, int begin, int end, int& sum) -> void {
    for (int i=begin; i<end; ++i) {
      sum += input_vec[i];
    }
  };
  std::vector<int> result(10, 0.0);
  thread_pool::ThreadPool tpool;
  for (int i=0; i<num_of_elem; i+= 10) {
    tpool.SubmitWork(std::bind(Addition, std::ref(input_vec), i, i+10, std::ref(result[i/10])));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  int total_sum = std::accumulate(result.begin(), result.end(), 0);
  EXPECT_EQ(total_sum, (num_of_elem - 1) * num_of_elem / 2);  // First elem is always zero so discaring the first elem
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}