#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iterator>
#include <numeric>
#include "src/tpool_waitabletask.h"

template <typename Iterator, typename T>
struct AccumulateBlock {
  T operator() (Iterator first, Iterator last) {
    return std::accumulate(first, last, T());
  }
};

template <typename Iterator, typename T>
T ParallelAccumulate(Iterator first, Iterator last, T init) {
  unsigned long const length = std::distance(first, last);
  if (!length) {
    return init;
  }
  unsigned long const block_size = 25;
  unsigned long const num_of_blocks = (length + block_size - 1) / block_size;
  std::vector<std::future<T>> futures(num_of_blocks - 1);
  thread_pool::TpoolWaitableTask tpool;
  Iterator block_start = first;
  for (unsigned long i=0; i<(num_of_blocks-1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    futures[i] = tpool.Submit(std::move(std::bind(AccumulateBlock<Iterator, T>(), block_start, block_end)));
    block_start = block_end;
  }
  T last_result = AccumulateBlock<Iterator, T>()(block_start, last);
  T result = init;
  for (unsigned long i=0; i<(num_of_blocks-1); ++i) {
    result += futures[i].get();
  }
  result += last_result;
  return result;
}

TEST (TpoolWaitableTask, RunParallelAccumulate) {
  const int num_of_items = 500;
  std::vector<int> input_vector(num_of_items);
  for (int i=0; i<num_of_items; i++) {
    input_vector[i] = i + 1;
  }
  EXPECT_EQ(ParallelAccumulate(input_vector.begin(), input_vector.end(), 0), num_of_items * (num_of_items + 1) / 2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}
