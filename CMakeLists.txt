cmake_minimum_required (VERSION 2.6)

cmake_policy(SET CMP0015 NEW)

set (CMAKE_CXX_STANDARD 11)

project (op1-utils)

include_directories(./include/)
link_directories(./lib/)

add_library(op1 op1_drum_impl.cpp)

add_executable(op1-dump op1-dump.cpp)
add_executable(op1-drum op1-drum.cpp)

target_link_libraries (op1-drum op1)
target_link_libraries (op1-drum -lsndfile)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
