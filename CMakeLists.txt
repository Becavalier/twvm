cmake_minimum_required (VERSION 2.6)
project (TWVM)

include_directories(.)

SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)

# Set c++ version;
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS_RELEASE "-O3")
set (OUTPUT "twvm")

aux_source_directory (. DIR_SRCS)

# add executable;
add_executable(${OUTPUT} ${DIR_SRCS})
