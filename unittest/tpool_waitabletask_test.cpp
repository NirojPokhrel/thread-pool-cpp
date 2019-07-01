#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iterator>
#include <numeric>
#include "src/tpool_waitabletask.h"

#if 0
#include <list>
template<typename T>
struct Sorter {
  thread_pool::TpoolWaitableTask tpool_;

  std::list<T> DoSort(std::list<T>& chunk_data) {
    if (chunk_data.empty()) {
      return chunk_data;
    }
    std::list<T> result;
    result.splice(result.begin(), chunk_data, chunk_data.begin());
    T const& partition_val = *result.begin();
    typename std::list<T>::iterator divide_point = std::partition(chunk_data.begin(), chunk_data.end(),
      [&](T const& val) {return val < partition_val;});
    std::list<T> new_lower_chunk;
    new_lower_chunk.splice(new_lower_chunk.end(), chunk_data, chunk_data.begin(), divide_point);
    std::future<std::list<T>> new_lower = tpool_.Submit(std::bind(&Sorter::DoSort, this, std::move(new_lower_chunk)));
    std::list<T> new_higher(DoSort(chunk_data));
    result.splice(result.end(), new_higher);
    while (new_lower.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
      //tpool_.run_pending_task();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    result.splice(result.begin(), new_lower.get());
    return result;
  }
};

template<typename T>
std::list<T> ParallelQuickSort(std::list<T> input) {
  if (input.empty()) {
    return input;
  }
  Sorter<T> s;
  return s.DoSort(input);
}


TEST (SimpleThreadPool, ParallelSort) {
  std::list<int> input{10, 50, 2, 7, 4, 3, 90, 1, 100, 10};
  auto result = ParallelQuickSort(input);
  for (auto res : result) {
    std::cout << res << std::endl;
  }
  EXPECT_TRUE (std::is_sorted(result.begin(), result.end()));
}
#endif
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
