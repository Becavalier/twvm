cmake_minimum_required (VERSION 3.11)

set (CMAKE_C_COMPILER /usr/bin/clang)
set (CMAKE_CXX_COMPILER /usr/bin/clang++)

project (TWVM)

include_directories(.)

# setup c++ config;
set (CMAKE_VERBOSE_MAKEFILE "OFF")
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} \
  -v       \
  -pthread \
  -O0      \
  -Wall    \
  -g       \
  -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} \
  -v       \
  -pthread \
  -Ofast   \
  -DNDEBUG \
  -Wall")

set (OUTPUT "twvm")
set (STATE $ENV{CMAKE_TARGET})
set (N_OUTPUT_SRC "bin")
set (T_OUTPUT_SRC "tests")

# VM options;
add_definitions(-DBUILD_VERSION="$ENV{BUILD_VERSION}")
if (DEFINED ENV{NDEBUG})
  add_definitions(-DNDEBUG)
endif()
add_definitions(-DOPT_DCT)

# print all definitions;
get_directory_property(DirDefs COMPILE_DEFINITIONS)
foreach(d ${DirDefs})
  message (STATUS "VM Define: " ${d})
endforeach()

if (STATE STREQUAL TEST)
  # download and unpack googletest at configure time;
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

  # prevent overriding the parent project's compiler/linker settings on Windows;
  set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory (${CMAKE_BINARY_DIR}/googletest-src
                ${CMAKE_BINARY_DIR}/googletest-build
                EXCLUDE_FROM_ALL)
  # resolve testing source files;
  aux_source_directory (./tests DIR_SRCS)
  set (EXECUTABLE_OUTPUT_PATH ${T_OUTPUT_SRC})
else()
  # resolve release source files;
  aux_source_directory (./src DIR_SRCS)
  set (EXECUTABLE_OUTPUT_PATH ${N_OUTPUT_SRC})
endif()  

# add subdirectory of the core library;
add_subdirectory (lib/structures)
add_subdirectory (lib/)

# add executable;
add_executable(${OUTPUT} ${DIR_SRCS})

if (STATE STREQUAL TEST) 
  target_link_libraries (${OUTPUT} 
    twvm-core
    twvm-structure
    gtest_main
    gtest)

  # run the test;
  add_custom_command(TARGET ${OUTPUT} POST_BUILD COMMAND ${OUTPUT})
else()
  # linker;
  target_link_libraries (${OUTPUT} 
    twvm-core
    twvm-structure
    pthread)

  # installing;
  install(TARGETS ${OUTPUT} RUNTIME DESTINATION bin)
endif()  
