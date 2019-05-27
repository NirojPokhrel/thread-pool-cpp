add_executable(ts_queue ts_queue_test.cpp)
target_link_libraries(ts_queue ${GTEST_STATIC_LIBRARIES} -pthread)