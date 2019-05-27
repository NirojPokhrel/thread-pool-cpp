set (GTEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/googletest)
set (GTEST_STATIC_LIBRARIES
      ${GTEST_SOURCE_DIR}/googlemock/libgmock_main.a
      ${GTEST_SOURCE_DIR}/googlemock/libgmock.a
      ${GTEST_SOURCE_DIR}/googlemock/gtest/libgtest_main.a
      ${GTEST_SOURCE_DIR}/googlemock/gtest/libgtest.a)

find_library (GTEST_LIBRARY
  NAMES   libgmock.a
  HINTS   "${CMAKE_SOURCE_DIR}/external/googletest/googlemock/"
)

function(gtestinit)
  include (${CMAKE_SOURCE_DIR}/cmake/gmock_download.cmake)
endfunction()

if (GTEST_LIBRARY)
  message(STATUS "GTest library - ${GTEST_LIBRARY}")
  # Check if the local copy of the library exists
  foreach(lib ${GTEST_STATIC_LIBRARIES})
    if (EXISTS ${lib})
      message(STATUS "Gtest Found - ${lib}")
    else ()
      gtestinit()
      break()
    endif ()    
  endforeach(lib ${})
else ()
  gtestinit()
endif ()

set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
set(GMOCK_INCLUDE_DIRS ${source_dir}/googlemock/include)
