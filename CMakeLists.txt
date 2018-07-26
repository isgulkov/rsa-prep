cmake_minimum_required(VERSION 3.10)
project(rsa_prep)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_library(longint src/longint.cpp)

add_executable(rsa_prep src/main.cpp)

target_link_libraries(rsa_prep longint)
