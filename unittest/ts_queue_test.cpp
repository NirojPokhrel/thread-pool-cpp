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

TEST (ThreadSafeQueueTest, PushPopOnEmptyQueue) {
  thread_pool::threadsafe_queue<int> q;
  std::promise<void> go, push_ready, pop_ready;
  std::shared_future<void> ready(go.get_future());
  std::future<void> push_done;
  std::future<int> pop_done;
  try {
    push_done = std::async(std::launch::async, [&q, ready, &push_ready]() {
      push_ready.set_value();
      ready.wait();
      q.push(42);
    });
    pop_done = std::async(std::launch::async, [&q, ready, &pop_ready]() {
      pop_ready.set_value();
      ready.wait();
      return *q.wait_and_pop();
    });
  } catch (...) {
    go.set_value();
    throw;
  }
  push_ready.get_future().wait();
  pop_ready.get_future().wait();
  go.set_value();
  push_done.get();
  EXPECT_EQ (pop_done.get(), 42);
  EXPECT_TRUE (q.empty());
}

TEST (ThreadSafeQueueTest, MultiThreadsPush) {
  thread_pool::threadsafe_queue<int> q;
  std::promise<void> go, push_ready1, push_ready2;
  std::shared_future<void> ready(go.get_future());
  std::future<void> push_done1, push_done2;
  try {
    push_done1 = std::async(std::launch::async, [&q, ready, &push_ready1] {
      push_ready1.set_value();
      ready.wait();
      q.push(42);
    });
    push_done2 = std::async(std::launch::async, [&q, ready, &push_ready2] {
      push_ready2.set_value();
      ready.wait();
      q.push(21);
    });
  } catch (...) {
    go.set_value();
    throw;
  }
  push_ready1.get_future().wait();
  push_ready2.get_future().wait();
  go.set_value();
  push_done1.get();
  push_done2.get();
  EXPECT_FALSE(q.empty());
  int val = *q.wait_and_pop();
  EXPECT_TRUE (val == 21 || val == 42);
  val = *q.wait_and_pop();
  EXPECT_TRUE (val == 21 || val == 42);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}
