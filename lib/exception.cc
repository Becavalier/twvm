// Copyright 2021 YHSPY. All rights reserved.
#include "lib/include/exception.hh"

namespace TWVM {

std::unordered_map<Exception::ErrorType, std::string> Exception::errorMsg = {
  { ErrorType::INVALID_MAGIC_CODE, "Invalid WebAssembly magic code found. " },
  { ErrorType::INVALID_VER_NUM, "Invalid WebAssembly version number found. " },
  { ErrorType::INVALID_SECTION_ID, "Invalid section id found. " },
  { ErrorType::INVALID_FUNC_TYPE, "Invalid function type found. " },
  { ErrorType::INVALID_ELEM_TYPE, "Invalid element type found, only `anyfunc` allowed in MVP. " },
  { ErrorType::BAD_FSTREAM, "Bad input file stream found. " },
  { ErrorType::INVALID_GLOBAL_SIG, "Invalid global signature found. " },
  { ErrorType::MEM_EXCEED_MAX, "The amount of inital memory pages must be less than maximum. " },
  { ErrorType::MEM_DATA_EXCEED_MAX, "The initialized data exceeds the size of memory instance. " },
  { ErrorType::TBL_EXCEED_MAX, "The amount of inital table entries must be less than maximum. " },
  { ErrorType::TBL_ELEM_EXCEED_MAX, "The amount of initialized elements exceeds the number of table entries. " },
  { ErrorType::MEM_ACCESS_OOB, "Memory address accessed out of bound. " },
  { ErrorType::TBL_ACCESS_OOB, "Table accessed out of bound. " },
  { ErrorType::MISSING_FUNC_PARAMS, "Insufficient values on stack for calling a function. " },
  { ErrorType::STACK_VAL_TYPE_MISMATCH, "Stack value type mismatch. " },
  { ErrorType::UNREACHABLE, "Unreachable. " },
  { ErrorType::ILLEGAL_LOCAL_IDX, "Illegal local index found. " },
  { ErrorType::EXHAUSTED_STACK_ACCESS, "No such available data on the stack. " },
  { ErrorType::ILLEGAL_BREAK_LVL, "Illegal Break level found. " },
  { ErrorType::ARITY_TYPE_MISMATCH, "Arity type(s) mismatch. " },
  { ErrorType::FUNC_TYPE_MISMATCH, "Function type mismatch. " },
  { ErrorType::NO_ACTIV_ON_STACK, "No available call frame on the stack. " },
  { ErrorType::ILLFORMED_STRUCTURE, "Ill-formed control structure found. " },
  { ErrorType::NO_AVAILABLE_TABLES_EXIST, "No available tables exist. " },
  { ErrorType::FUNC_TYPE_ACCESS_OOB, "Function types accessed out of bound. " },
  { ErrorType::GLOBAL_ACCESS_OOB, "Global instance accessed out of bound. " },
  { ErrorType::IMMUTABLE_GLOBAL_MUTATION, "Immutable global cannot be changed. " },
  { ErrorType::NO_AVAILABLE_MEM, "No available memory instances exist. " },
  { ErrorType::DIVISION_BY_ZERO, "Division by zero found. " },
  { ErrorType::VAL_NOT_REPRESENTABLE, "Result is not representable. " },
  { ErrorType::FLOAT_UNREPRESENTABLE, "Floating point number unrepresentable in integer range. " },
  { ErrorType::INVALID_CONVERSION_STOI, "Invalid conversion from string to integer found. " },
  { ErrorType::INVALID_INPUT_PATH, "Invalid path of the input file found. " },
  { ErrorType::INSUFFICIENT_INPUT_ARGS, "Insufficient arguments for invoking the function." },
  { ErrorType::INVALID_VAL_TYPE, "Invalid WebAssembly value type found." },
};

}  // namespace TWVM
