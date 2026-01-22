# Find LLVM package
# Try Homebrew path first (macOS)
if(NOT CMAKE_PREFIX_PATH AND EXISTS "/opt/homebrew/opt/llvm")
  set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/llvm")
endif()
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# LLVM components needed for JIT
llvm_map_components_to_libnames(llvm_libs
  Core
  OrcJIT
  Support
  native
)

aux_source_directory(. DIR_LIB_SRCS)
add_library(twvm-core ${DIR_LIB_SRCS})

# Link against LLVM libraries
target_link_libraries(twvm-core ${llvm_libs})
