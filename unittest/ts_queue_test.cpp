#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "src/threadsafe_queue.h"

TEST (FirstTest, TestSomething) {

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*";
  return RUN_ALL_TESTS();
}