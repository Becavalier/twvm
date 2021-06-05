#include "lib/exception.h"

std::unordered_map<Exception::ErrorType, std::string> Exception::errorMsg = {
  { ErrorType::INVALID_MAGIC, "Invalid binary magic code found. " },
  { ErrorType::INVALID_VER, "Invalid WebAssembly version code found. " },
  { ErrorType::INVALID_SECTION_ID, "Invalid Section id found. " },
  { ErrorType::INVALID_FUNC_TYPE, "Invalid Function type found. " },
  { ErrorType::INVALID_ELEM_TYPE, "Invalid element type found, only `anyfunc` allowed in MVP. " },
  { ErrorType::BAD_FSTREAM, "Bad input file stream found. " },
};
