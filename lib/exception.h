// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXCEPTION_H_
#define LIB_EXCEPTION_H_

#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <stdexcept>

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
  };
  [[noreturn]]
  static void terminate(ErrorType type, ssize_t pos = 0) {
    std::stringstream ss;
    ss << std::hex << std::showbase
      << "\n[twvm] " 
      << errorMsg.find(type)->second;
    if (pos) {
      ss << "Before byte index: " << pos  << '.';
    }  
    ss << std::endl;
    throw std::runtime_error(ss.str());
  }
 private:
  static std::unordered_map<ErrorType, std::string> errorMsg;
};

#endif  // LIB_EXCEPTION_H_
