cmake_minimum_required(VERSION 3.10)
project(rsa_prep)

set(CMAKE_CXX_STANDARD 11)

### Libraries
### ---------
include_directories(include)

### longint: a long integer implementation
add_library(longint src/longint.cpp)

### CLI executable, perhaps?
### ------------------------
add_executable(rsa_prep src/main.cpp)

target_link_libraries(rsa_prep longint)

### Unit tests
### ----------
option(BUILD_TESTS "Build all tests." OFF)

if (BUILD_TESTS)
  add_subdirectory(vendor/gtest)

  enable_testing()

  # - longint
  add_executable(test_longint test/test_longint.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_longint.cpp")
  target_link_libraries(test_longint
          gtest gtest_main
          longint
  )
  add_test(longint test_longint)

  # all tests
  # TODO: a more concise way?
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          longint
  )
endif()
