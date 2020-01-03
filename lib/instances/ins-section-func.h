// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_SECTION_FUNC_H_
#define LIB_INSTANCES_INS_SECTION_FUNC_H_

#include <vector>
#include <memory>
#include "lib/type.h"
#include "lib/common/macros.h"

using std::vector;
using std::shared_ptr;

struct WasmFuncInstance {
  SET_STRUCT_MOVE_ONLY(WasmFuncInstance);
  WasmFunctionSig* type;
  WasmFunction* staticProto;
  vector<uint8_t> *code;
};

#endif  // LIB_INSTANCES_INS_SECTION_FUNC_H_
