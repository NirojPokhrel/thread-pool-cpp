#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "src/threadsafe_queue.h"

TEST (ThreadSafeQueueTest, TryPopEmpty) {
  thread_pool::threadsafe_queue<int> ts_queue;
  int32_t val;
  EXPECT_FALSE(ts_queue.try_pop(val));
  EXPECT_EQ(nullptr, ts_queue.try_pop());
}

TEST (ThreadSafeQueueTest, WaitAndPop) {
  thread_pool::threadsafe_queue<int> ts_queue;
  std::future<int> fut = std::async(std::launch::async, [&ts_queue]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));    // sleep and let main thread wait
    int elem = 10;
    ts_queue.push(elem);
    return elem;
  });
  int val;
  ts_queue.wait_and_pop(val);
  EXPECT_EQ(fut.get(), val);
}

TEST (ThreadSafeQueueTest, MutliThreadInsert) {
  thread_pool::threadsafe_queue<int> ts_queue;
  std::future<void> fut1 = std::async(std::launch::async, [&ts_queue](){

    for (auto i : {10, 15, 20}) {
      ts_queue.push(i);
    }
  });
  std::future<void> fut2 = std::async(std::launch::async, [&ts_queue](){
    for (auto i : {25, 30, 35}) {
      ts_queue.push(i);
    }
  });
  fut1.get();   // insert all the elements from first thread
  fut2.get();   // insert all the elements from second thread
  int i = 0;
  while (i++ < 5) {
    ts_queue.try_pop();
    EXPECT_FALSE(ts_queue.empty());
  }
  ts_queue.try_pop();
  EXPECT_TRUE(ts_queue.empty());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}