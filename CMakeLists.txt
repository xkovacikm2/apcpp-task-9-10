cmake_minimum_required(VERSION 3.6)
project(9_10_zadanie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

set(SOURCE_FILES main.cpp sha1.cpp sha1.hpp)
add_executable(9_10_zadanie ${SOURCE_FILES})