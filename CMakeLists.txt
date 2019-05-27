cmake_minimum_required(VERSION 3.5)

project (thread_pool CXX C)

set (CMAKE_CXX_STANDARD 11)

# set up the google test for unittest
include (cmake/gmock_init.cmake)
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory("unittest")
