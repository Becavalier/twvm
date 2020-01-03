// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_SECTION_TABLE_H_
#define LIB_INSTANCES_INS_SECTION_TABLE_H_

#include <vector>
#include "lib/type.h"
#include "lib/common/macros.h"

using std::vector;

struct WasmTableInstance {
  SET_STRUCT_MOVE_ONLY(WasmTableInstance);
  uint32_t maxTableSize;
  vector<WasmFunction*> funcIndices;
};

#endif  // LIB_INSTANCES_INS_SECTION_TABLE_H_
