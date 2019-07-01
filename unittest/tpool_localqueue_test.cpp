#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <list>

#include "src/tpool_localqueue.h"

template<typename T>
struct Sorter {
  thread_pool::ThreadPoolLocalWorkQueue tpool_;

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
      tpool_.RunPendingTask();
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}