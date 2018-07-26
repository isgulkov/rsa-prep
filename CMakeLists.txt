cmake_minimum_required(VERSION 3.10)
project(rsa_prep)

set(CMAKE_CXX_STANDARD 11)

### Libraries
### ---------
include_directories(include)

# - uint_long: an unsigned long integer implementation
add_library(uint_long src/uint_long.cpp)

### CLI executable, perhaps?
### ------------------------
add_executable(rsa_prep src/main.cpp)

target_link_libraries(rsa_prep uint_long)

### Unit tests
### ----------
option(BUILD_TESTS "Build all tests." OFF)

if (BUILD_TESTS)
  add_subdirectory(vendor/gtest)

  enable_testing()

  # - uint_long
  add_executable(test_uint_long test/test_uint_long.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_uint_long.cpp")
  target_link_libraries(test_uint_long
          gtest gtest_main
          uint_long
  )
  add_test(test_uint_long test_uint_long)

  # all tests
  # TODO: a more concise way?
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          uint_long
  )
endif()
