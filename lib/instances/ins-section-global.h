// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_SECTION_GLOBAL_H_
#define LIB_INSTANCES_INS_SECTION_GLOBAL_H_

#include "lib/type.h"
#include "lib/common/macros.h"

struct WasmGlobalInstance {
  SET_STRUCT_MOVE_ONLY(WasmGlobalInstance);
  ValueTypesCode type;
  RTValue val;
  bool mutability;
};

#endif  // LIB_INSTANCES_INS_SECTION_GLOBAL_H_
