add_executable(ts_queue ts_queue_test.cpp)
target_link_libraries(ts_queue ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(s_tpool ts_threadpool_test.cpp)
target_link_libraries(s_tpool ${GTEST_STATIC_LIBRARIES} -pthread)