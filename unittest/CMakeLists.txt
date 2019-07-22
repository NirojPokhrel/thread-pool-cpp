add_executable(ts_queue ts_queue_test.cpp)
target_compile_options(ts_queue PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(ts_queue ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(s_tpool ts_threadpool_test.cpp)
target_compile_options(s_tpool PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(s_tpool ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(tpool_waitabletask tpool_waitabletask_test.cpp)
target_compile_options(tpool_waitabletask PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(tpool_waitabletask ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(tpool_localqueue tpool_localqueue_test.cpp)
target_compile_options(tpool_localqueue PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(tpool_localqueue ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(work_stealing_queue work_stealing_queue_test.cpp)
target_compile_options(work_stealing_queue PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(work_stealing_queue ${GTEST_STATIC_LIBRARIES} -pthread)

add_executable(tpool_work_stealing tpool_worksteal_test.cpp)
target_compile_options(tpool_work_stealing PUBLIC ${LOCAL_BUILD_FLAGS})
target_link_libraries(tpool_work_stealing ${GTEST_STATIC_LIBRARIES} -pthread)
