cmake_minimum_required(VERSION 3.10)
project(rsa_prep
        VERSION 0.1
        DESCRIPTION "Crypto of the own-rolled kind"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall")
#                          ^^^
# This may or may not improve llvm's "Evaluate Expression" functionality in CLion

### Libraries
### ---------
include_directories(include)

# - intbig_t: a multiple-precision integer implementation
add_library(intbig_t src/intbig_t.cpp)

# REMOVE: for converting to std::string during development
target_include_directories(intbig_t PRIVATE vendor/infint)

# - sha256: a SHA256 cryptograpihc hash implementation
add_library(sha256 src/sha256.cpp)

### CLI executable, perhaps?
### ------------------------
add_executable(rsa_prep src/main.cpp)
# REMOVE: v
target_link_libraries(rsa_prep sha256)
# REMOVE: ^

### Unit tests
### ----------
option(BUILD_TESTS "Build all tests." OFF)

if (BUILD_TESTS)
  add_subdirectory(vendor/gtest)

  enable_testing()

  # - intbig_t
  add_executable(test_intbig_t_additive test/test_intbig_t_additive.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_additive.cpp")
  target_link_libraries(test_intbig_t_additive
          gtest gtest_main
          intbig_t
          )
  target_include_directories(test_intbig_t_additive PRIVATE vendor/infint)
  add_test(test_intbig_t_additive test_intbig_t_additive)

  add_executable(test_intbig_t_comparisons test/test_intbig_t_comparisons.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_comparisons.cpp")
  target_link_libraries(test_intbig_t_comparisons
          gtest gtest_main
          intbig_t
          )
  add_test(test_intbig_t_comparisons test_intbig_t_comparisons)

  # all tests
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          intbig_t
  )
  target_include_directories(test_all PRIVATE vendor/infint)

  # TODO: https://stackoverflow.com/a/28305481:
  # TODO:   make sense of what the PRIVATE and PUBLIC do in `target_include_directories`
endif()
