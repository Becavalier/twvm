#include "lib/exception.h"

std::unordered_map<Exception::ErrorType, std::string> Exception::errorMsg = {
  { ErrorType::INVALID_MAGIC, "Invalid binary magic code found. " },
  { ErrorType::INVALID_VER, "Invalid WebAssembly version code found. " },
  { ErrorType::INVALID_SECTION_ID, "Invalid Section id found. " },
  { ErrorType::INVALID_FUNC_TYPE, "Invalid Function type found. " },
  { ErrorType::INVALID_ELEM_TYPE, "Invalid element type found, only `anyfunc` allowed in MVP. " },
  { ErrorType::BAD_FSTREAM, "Bad input file stream found. " },
  { ErrorType::INVALID_GLOBAL_SIG, "Invalid global signature (global_type and init_expr) found. " },
  { ErrorType::MEM_EXCEED_MAX, "The amount of inital memory pages must be less than maximum. " },
  { ErrorType::MEM_DATA_EXCEED_MAX, "The initialized data exceed the memory size. " },  
  { ErrorType::TBL_EXCEED_MAX, "The amount of inital table entries must be less than maximum. " },
  { ErrorType::TBL_ELEM_EXCEED_MAX, "The initialized elements exceed the table entries. " },  
  { ErrorType::MEM_ACCESS_OOB, "Memory access out of bound. " },  
  { ErrorType::TBL_ACCESS_OOB, "Table access out of bound. " },  
};