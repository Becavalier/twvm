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
  };
  [[noreturn]]
  static void terminate(ErrorType type, size_t pos = 0) {
    std::stringstream ss;
    ss << std::hex << std::showbase
      << "\n[TWVM] " 
      << errorMsg.find(type)->second 
      << "At byte index: "
      << pos 
      << '.' 
      << std::endl;
    throw std::runtime_error(ss.str());
  }
 private:
  static std::unordered_map<ErrorType, std::string> errorMsg;
};

#endif  // LIB_EXCEPTION_H_
