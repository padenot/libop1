cmake_minimum_required (VERSION 2.6)

set (CMAKE_CXX_STANDARD 11)

project (op1-utils)

include_directories(./include/)
link_directories(./lib/)

add_library(libop1 op1_drum_impl.cpp)

add_executable(op1-dump op1-dump.cpp)
add_executable(op1-drum op1-drum.cpp)

target_link_libraries (op1-drum libop1)
target_link_libraries (op1-drum -lsndfile)

