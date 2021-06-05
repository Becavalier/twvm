#include "lib/exception.h"

std::unordered_map<Exception::ErrorType, std::string> Exception::errorMsg = {
  { ErrorType::INVALID_MAGIC, "Invalid binary magic code found. " },
  { ErrorType::INVALID_VER, "Invalid WebAssembly version code found. " },
  { ErrorType::INVALID_SECTION_ID, "Invalid Section id found. " },
};
