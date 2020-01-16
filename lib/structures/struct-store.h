// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_STRUCTURES_STRUCT_STORE_H_
#define LIB_STRUCTURES_STRUCT_STORE_H_

#include <vector>
#include "lib/common/macros.h"
#include "lib/instances/ins-section-memory.h"
#include "lib/instances/ins-section-func.h"
#include "lib/instances/ins-section-global.h"
#include "lib/instances/ins-section-table.h"

using std::vector;

// for storing global instances, it consists of -
// the runtime representation of all instances of functions, tables, memories, and globals -
// that have been allocated during the life time of the VM.
struct Store {
  SET_STRUCT_MOVE_ONLY(Store);
  vector<WasmMemoryInstance> memoryInsts;
  vector<WasmFuncInstance> functionInsts;
  vector<WasmGlobalInstance> globalInsts;
  vector<WasmTableInstance> tableInsts;
};

#endif  // LIB_STRUCTURES_STRUCT_STORE_H_
