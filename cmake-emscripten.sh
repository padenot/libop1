#!/bin/sh

# WIP, use cmake to generate the emscripten makefiles

cmake -DCMAKE_TOOLCHAIN_FILE=Emscripten.cmake -DCMAKE_BUILD_TYPE=Debug #|RelWithDebInfo|Release|MinSizeRel> -G "Unix Makefiles" (Linux and OSX) -G "MinGW Makefiles" (Windows) ./CMakeLists.txt
