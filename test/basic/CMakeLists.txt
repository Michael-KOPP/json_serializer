cmake_minimum_required(VERSION 3.15)

add_executable(basic serialization.cpp)

target_link_libraries(basic PRIVATE gtest gtest_main serialization)

