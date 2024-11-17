cmake_minimum_required(VERSION 3.13)
project(gabby)
file(GLOB_RECURSE SOURCE_FILES src/*.cc src/*.h)
set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(gabby ${SOURCE_FILES})
