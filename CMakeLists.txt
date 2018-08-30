cmake_minimum_required(VERSION 3.10)
project(rsa_prep
        VERSION 0.1
        DESCRIPTION "Crypto of the own-rolled kind"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall")
#                          ^^^
# This may or may not improve llvm's "Evaluate Expression" functionality in CLion

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -v")

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
  add_executable(test_intbig_t_comparisons test/test_intbig_t_comparisons.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_comparisons.cpp")
  target_link_libraries(test_intbig_t_comparisons
          gtest gtest_main
          intbig_t
          )
  add_test(test_intbig_t_comparisons test_intbig_t_comparisons)

  add_executable(test_intbig_t_add_bin test/test_intbig_t_add_bin.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_add_bin.cpp")
  target_link_libraries(test_intbig_t_add_bin
          gtest gtest_main
          intbig_t
          )
  target_include_directories(test_intbig_t_add_bin PRIVATE vendor/infint)
  add_test(test_intbig_t_add_bin test_intbig_t_add_bin)

  add_executable(test_intbig_t_add_un test/test_intbig_t_add_un.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_add_un.cpp")
  target_link_libraries(test_intbig_t_add_un
          gtest gtest_main
          intbig_t
          )
  target_include_directories(test_intbig_t_add_un PRIVATE vendor/infint)
  add_test(test_intbig_t_add_un test_intbig_t_add_un)

  add_executable(test_intbig_t_shifts test/test_intbig_t_shifts.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_shifts.cpp")
  target_link_libraries(test_intbig_t_shifts
          gtest gtest_main
          intbig_t
          )
  target_include_directories(test_intbig_t_shifts PRIVATE vendor/infint)
  add_test(test_intbig_t_shifts test_intbig_t_shifts)

  add_executable(test_intbig_t_bitwise test/test_intbig_t_bitwise.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_bitwise.cpp")
  target_link_libraries(test_intbig_t_bitwise
          gtest gtest_main
          intbig_t
          )
  target_include_directories(test_intbig_t_bitwise PRIVATE vendor/mini-gmp-6.1.2)
  add_test(test_intbig_t_bitwise test_intbig_t_bitwise)

  # - sha256
  add_executable(test_sha256 test/test_sha256.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_sha256.cpp")
  target_link_libraries(test_sha256
          gtest gtest_main
          sha256
          )
  add_test(test_sha256 test_sha256)

  # all tests
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          intbig_t sha256
  )
  target_include_directories(test_all PRIVATE vendor/infint vendor/mini-gmp-6.1.2)

  # TODO: https://stackoverflow.com/a/28305481:
  # TODO:   make sense of what the PRIVATE and PUBLIC do in `target_include_directories`
endif()
