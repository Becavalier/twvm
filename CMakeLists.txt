cmake_minimum_required (VERSION 3.11)

project (TWVM)

include_directories(.)

# set (CMAKE_C_COMPILER /usr/bin/clang)
# set (CMAKE_CXX_COMPILER /usr/bin/clang++)

# set c++ config;
set (CMAKE_VERBOSE_MAKEFILE "OFF")
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

set (OUTPUT "twvm")
set (STATE $ENV{CMAKE_TARGET})
set (N_OUTPUT_SRC "build")
set (T_OUTPUT_SRC "test")

if (STATE STREQUAL TEST)
  set (EXECUTABLE_OUTPUT_PATH ${T_OUTPUT_SRC})
  # download and unpack googletest at configure time
  configure_file (CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process (COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

  if (result)
      message (FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()

  execute_process (COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

  if (result)
      message (FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows;
  set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory (${CMAKE_BINARY_DIR}/googletest-src
                ${CMAKE_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)
  # resolve source files;
  aux_source_directory (${T_OUTPUT_SRC} DIR_SRCS)
else()
  set (EXECUTABLE_OUTPUT_PATH ${N_OUTPUT_SRC})
  # resolve source files;
  aux_source_directory (./src DIR_SRCS)
endif()  

# add executable;
add_executable(${OUTPUT} ${DIR_SRCS})

install(TARGETS ${OUTPUT} RUNTIME DESTINATION bin)

if (STATE STREQUAL TEST) 
  target_link_libraries (${OUTPUT} gtest_main gtest)
  # run the test;
  add_custom_command(TARGET ${OUTPUT} POST_BUILD
      COMMAND ${OUTPUT})
endif()  
