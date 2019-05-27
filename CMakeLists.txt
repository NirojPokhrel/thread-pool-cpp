cmake_minimum_required(VERSION 3.5)

# set up the google test for unittest
include (cmake/gmock_init.cmake)

add_subdirectory("unittest")