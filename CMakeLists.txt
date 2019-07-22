cmake_minimum_required(VERSION 3.0)

project (thread_pool CXX C)

set (CMAKE_CXX_STANDARD 11)

message (STATUS "Cmake system ${CMAKE_SYSTEM}")

list (APPEND BUILD_FLAGS  "-Wall"
                          "-Wextra"
                          "-Wunused"
                          "-Wunintialized"
                          "-Wmissing-noreturn"
                          "-Wunreachable-code"
                          "-Wsign-compare"
                          "-Wstrict-null-sentinel"
                          "-Wreturn-type"
                          "-Wcast-qual"
                          "-Wconversion"
                          "-Wenum-compare"
                          "-Wsign-conversion")
# set up the google test for unittest
include (cmake/gmock_init.cmake)
include_directories (${GTEST_INCLUDE_DIRS})
include_directories (${GMOCK_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory("unittest")
enable_testing()
add_test(
  NAME ts_queue
  COMMAND ${CMAKE_BINARY_DIR}/unittest/ts_queue
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)

add_test(
  NAME s_tpool
  COMMAND ${CMAKE_BINARY_DIR}/unittest/s_tpool
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)

add_test(
  NAME tpool_waitabletask
  COMMAND ${CMAKE_BINARY_DIR}/unittest/tpool_waitabletask
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)

add_test(
  NAME tpool_localqueue
  COMMAND ${CMAKE_BINARY_DIR}/unittest/tpool_localqueue
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)

add_test(
  NAME work_stealing_queue
  COMMAND ${CMAKE_BINARY_DIR}/unittest/work_stealing_queue
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)

add_test(
  NAME tpool_work_stealing
  COMMAND ${CMAKE_BINARY_DIR}/unittest/tpool_work_stealing
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittest
)
