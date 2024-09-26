cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
project(serialization)

option(BUILD_TESTS ON)

add_subdirectory(src)

# if(BUILD_TESTS STREQUAL "ON")
    add_subdirectory(test)
# endif()

