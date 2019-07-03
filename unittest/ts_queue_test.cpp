#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <future>

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

TEST (ThreadSafeQueueTest, MutltiThreadPopWithInsufficientData) {
  thread_pool::threadsafe_queue<int> q;
  q.push(20);
  std::promise<void> go, pop_ready[2];
  std::shared_future<void> ready(go.get_future());
  std::future<std::shared_ptr<int>> push_done[2];
  try {
    for (int i=0; i<sizeof(pop_ready)/sizeof(pop_ready[0]); ++i) {
      push_done[i] = std::async(std::launch::async, [&q, ready, &pop_ready, i] {
        pop_ready[i].set_value();
        ready.wait();
        return q.try_pop();
      });
    }
  } catch (...) {
    go.set_value();
    throw;
  }
  for (int i=0; i<sizeof(pop_ready)/sizeof(pop_ready[0]); ++i) {
    pop_ready[i].get_future().wait();
  }
  go.set_value();
  auto data1 = push_done[0].get();
  auto data2 = push_done[1].get();
  EXPECT_TRUE (data1 == nullptr || *data1 == 20);
  EXPECT_TRUE (data2 == nullptr || *data2 == 20);
}

TEST (ThreadSafeQueueTest, MultiPushMutliPop) {
  thread_pool::threadsafe_queue<int> q;
  std::promise<void> go, pop_ready[2], push_ready[2];
  std::shared_future<void> ready(go.get_future());
  std::future<void> push_done[2];
  std::future<std::shared_ptr<int>> pop_done[2];
  try {
    int value = 21;
    for (int i=0; i<sizeof(push_ready)/sizeof(push_ready[0]); i++) {
      push_done[i] = std::async(std::launch::async, [&q, ready, &push_ready, i]{
        push_ready[i].set_value();
        ready.wait();
        q.push((i+1) * 21);
      });
    }
    for (int i=0; i<sizeof(pop_ready)/sizeof(pop_ready[0]); i++) {
      pop_done[i] = std::async(std::launch::async, [&q, ready, &pop_ready, i] {
        pop_ready[i].set_value();
        ready.wait();
        return q.wait_and_pop();
      });
    }
  } catch (...) {
    go.set_value();
    throw;
  }
  for (int i=0; i<sizeof(push_ready)/sizeof(push_ready[0]); i++) {
    push_ready[i].get_future().get();
  }
  for (int i=0; i<sizeof(push_ready)/sizeof(push_ready[0]); i++) {
    pop_ready[i].get_future().get();
  }
  go.set_value();
  auto ptr1 = pop_done[0].get();
  auto ptr2 = pop_done[1].get();
  EXPECT_TRUE (*ptr1 == 21 || *ptr1 == 42);
  EXPECT_TRUE (*ptr2 == 21 || *ptr2 == 42);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}
