cmake_minimum_required(VERSION 3.10)
project(rsa_prep
        VERSION 0.1
        DESCRIPTION "Crypto of the own-rolled kind"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall -Wextra")
#                          ^^^
# This may or may not improve lldb's "Evaluate Expression" functionality in CLion

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -v")

set(EXECUTABLE_OUTPUT_PATH ../apps)

### Libraries
### ---------
include_directories(include)

# - intbig_t: a multiple-precision integer implementation
add_library(intbig_t src/intbig_t.cpp)

# - primes: generation of large random primes
add_library(primes src/primes.cpp)
target_link_libraries(primes PUBLIC intbig_t)

# - sha256: a SHA256 cryptograpihc hash implementation
add_library(sha256 src/sha256.cpp)

# - base64: a base64 codec as defined in RFC4880
add_library(base64 src/base64.cpp)

# - rsa: RSA cryptosystem
add_library(rsa src/rsa.cpp)
target_link_libraries(rsa PRIVATE intbig_t primes sha256)

# - formats: file formats resembling OpenPGP's ASCII-armored DER
add_library(formats src/formats.cpp)
target_link_libraries(formats rsa base64)

### CLI applications
### ----------------

# - sha256sum: a file hashing utility like shasum
add_executable(sha256sum apps/sha256sum.cpp)
target_link_libraries(sha256sum sha256)

# - b64codec: a command-line base64 codec
add_executable(b64codec apps/b64codec.cpp)
target_link_libraries(b64codec base64)

# - prime: large prime generation utility
add_executable(prime apps/prime.cpp)
target_link_libraries(prime primes)

# - igpg: cryptography tool
add_executable(igpg apps/igpg.cpp)
target_link_libraries(igpg rsa formats)

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
  add_test(test_intbig_t_add_bin test_intbig_t_add_bin)

  add_executable(test_intbig_t_add_bin1 test/test_intbig_t_add_bin1.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_intbig_t_add_bin1.cpp")
  target_link_libraries(test_intbig_t_add_bin1
          gtest gtest_main
          intbig_t
          )
  add_test(test_intbig_t_add_bin1 test_intbig_t_add_bin1)

  # - sha256
  add_executable(test_sha256 test/test_sha256.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_sha256.cpp")
  target_link_libraries(test_sha256
          gtest gtest_main
          sha256
          )
  add_test(test_sha256 test_sha256)

  # - base64
  add_executable(test_base64 test/test_base64.cpp)
  set(TEST_SRCS "${TEST_SRCS};test/test_base64.cpp")
  target_link_libraries(test_base64
          gtest gtest_main
          base64
          )
  add_test(test_base64 test_base64)

  # all tests
  add_executable(test_all test/main.cpp "${TEST_SRCS}")
  target_link_libraries(test_all
          gtest gtest_main
          intbig_t sha256 base64
  )

  # TODO: https://stackoverflow.com/a/28305481:
  # TODO:   make sense of what the PRIVATE and PUBLIC do in `target_include_directories`
endif()
