#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <future>
#include <iterator>
#include <list>

#include "include/tpool_worksteal.h"


TEST (WorkStealTpool, SubmitDifferentTask) {
  int a = 5, b = 10, c = 15, d = 20;
  auto Addition = [](int x, int y, int& res) -> void {
    for (int i=0; i<y; i++) {
      res += x;
      std::this_thread::yield();  // let another thread run after each iteration
    }
  };
  int e = 0, f = 0;
  thread_pool::WorkStealTpool tpool;
  auto fut1 = tpool.SubmitWork(std::bind(Addition, a, b, std::ref(e)));   // std::ref is required to let bind operation know it is references
  auto fut2 = tpool.SubmitWork(std::bind(Addition, c, d, std::ref(f)));   // otherwise, bind operations copies the variable and this copy's reference is passed to the lambda
  fut1.get();
  fut2.get();
  EXPECT_EQ(e, 50);
  EXPECT_EQ(f, 300);
}

TEST (WorkStealTpool, ParallelAccumulateTenElem) {
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
  thread_pool::WorkStealTpool tpool;
  std::vector<std::future<void>> fut_vec;
  for (int i=0; i<num_of_elem; i+= 10) {
    auto fut = tpool.SubmitWork(std::bind(Addition, std::ref(input_vec), i, i+10, std::ref(result[i/10])));
    fut_vec.push_back(std::move(fut));
  }
  for (int i=0; i<fut_vec.size(); ++i) {
    fut_vec[i].get();
  }
  int total_sum = std::accumulate(result.begin(), result.end(), 0);
  EXPECT_EQ(total_sum, (num_of_elem - 1) * num_of_elem / 2);  // First elem is always zero so discaring the first elem
}

template<typename T>
struct Sorter {
  thread_pool::WorkStealTpool tpool_;

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
    std::future<std::list<T>> new_lower = tpool_.SubmitWork(std::bind(&Sorter::DoSort, this, std::move(new_lower_chunk)));
    std::list<T> new_higher(DoSort(chunk_data));
    result.splice(result.end(), new_higher);
#if 0
    while (new_lower.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
      // tpool_.RunPendingTask();
    }
#endif
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


TEST (WorkStealTpool, ParallelSort) {
  std::list<int> input{10, 50, 2, 7, 4, 3, 90, 1, 100, 10};
  auto result = ParallelQuickSort(input);
  EXPECT_TRUE (std::is_sorted(result.begin(), result.end()));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}