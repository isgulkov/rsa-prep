cmake_minimum_required(VERSION 3.10)
project(rsa_prep)

set(CMAKE_CXX_STANDARD 11)

### Libraries
### ---------
include_directories(include)

# - uint_long: an unsigned long integer implementation
add_library(uint_long src/uint_long.cpp)

# REMOVE: for converting to std::string during development
target_include_directories(uint_long PRIVATE vendor/infint)

### CLI executable, perhaps?
### ------------------------
add_executable(rsa_prep src/main.cpp)

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
  target_include_directories(test_uint_long PRIVATE vendor/infint)
  add_test(test_uint_long test_uint_long)

  # all tests
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          uint_long
  )
  # TODO: can I make every single test a library? So that `test_*` links to one of them and this one to all?
  # TODO: https://cmake.org/cmake/help/v3.5/command/add_library.html#id6
  # TODO: https://cmake.org/cmake/help/v3.5/manual/cmake-buildsystem.7.html#interface-libraries

  # TODO: https://stackoverflow.com/a/28305481:
  # TODO:   make sense of what the PRIVATE and PUBLIC do in `target_include_directories`
endif()
