cmake_minimum_required(VERSION 3.10)
project(rsa_prep
        VERSION 0.1
        DESCRIPTION "Crypto of the own-rolled kind"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

### Libraries
### ---------
include_directories(include)

# - intbig_t: a multiple-precision integer implementation
add_library(intbig_t src/intbig_t.cpp)

# REMOVE: for converting to std::string during development
target_include_directories(intbig_t PRIVATE vendor/infint)

### CLI executable, perhaps?
### ------------------------
add_executable(rsa_prep src/main.cpp)
target_link_libraries(rsa_prep intbig_t)

### Unit tests
### ----------
option(BUILD_TESTS "Build all tests." OFF)

if (BUILD_TESTS)
  add_subdirectory(vendor/gtest)

  enable_testing()

  # - intbig_t
  add_executable(test_intbig_t test/test_intbig_t.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t.cpp")
  target_link_libraries(test_intbig_t
          gtest gtest_main
          intbig_t
  )
  target_include_directories(test_intbig_t PRIVATE vendor/infint)
  add_test(test_intbig_t test_intbig_t)

  # all tests
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          intbig_t
  )
  # TODO: can I make every single test a library? So that `test_*` links to one of them and this one to all?
  # TODO: https://cmake.org/cmake/help/v3.5/command/add_library.html#id6
  # TODO: https://cmake.org/cmake/help/v3.5/manual/cmake-buildsystem.7.html#interface-libraries

  # TODO: https://stackoverflow.com/a/28305481:
  # TODO:   make sense of what the PRIVATE and PUBLIC do in `target_include_directories`
endif()
