// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXCEPTION_H_
#define LIB_EXCEPTION_H_

#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include "lib/include/constants.hh"

class Exception {
 public:
  enum class ErrorType : uint8_t {
    INVALID_MAGIC,
    INVALID_VER,
    INVALID_SECTION_ID,
    INVALID_FUNC_TYPE,
    INVALID_ELEM_TYPE,
    BAD_FSTREAM,
    INVALID_GLOBAL_SIG,
    MEM_EXCEED_MAX,
    MEM_DATA_EXCEED_MAX,
    MEM_ACCESS_OOB,
    TBL_EXCEED_MAX,
    TBL_ELEM_EXCEED_MAX,
    TBL_ACCESS_OOB,
    MISSING_FUNC_PARAMS,
    STACK_VAL_TYPE_MISMATCH,
    UNREACHABLE,
    ILLEGAL_LOCAL_IDX,
    EXHAUSTED_STACK_ACCESS,
    BREAK_LEVEL_EXCEEDED,
    ARITY_TYPE_MISMATCH,
    NO_ACTIV_ON_STACK,
    ILLFORMED_STRUCTURE,
    NO_AVAILABLE_TABLES_EXIST,
    FUNC_TYPE_ACCESS_OOB,
    GLOBAL_ACCESS_OOB,
    FUNC_TYPE_MISMATCH,
    IMMUTABLE_GLOBAL_CHANGED,
    NO_AVAILABLE_MEM,
    DIVISION_BY_ZERO,
    VAL_NOT_REPRESENTABLE,
    FLOAT_UNREPRESENTABLE,
    INVALID_CONVERSION_STOI,
    INVALID_INPUT_PATH,
  };
  [[noreturn]]
  static void terminate(ErrorType type, ssize_t pos = 0) {
    std::stringstream ss;
    ss << std::hex << std::showbase
      << COLOR_ERR
      << "[twvm] " 
      << RESET_NORMAL
      << errorMsg.at(type);
    if (pos) {
      ss << "Byte index before: " << pos  << '.';
    }  
    std::cerr << ss.str() << std::endl;
    std::exit(EXIT_FAILURE);
  }
 private:
  static std::unordered_map<ErrorType, std::string> errorMsg;
};

#endif  // LIB_EXCEPTION_H_
