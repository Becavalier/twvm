// Copyright 2019 YHSPY. All rights reserved.
#ifndef STORE_H_
#define STORE_H_

#include <vector>
#include "lib/include/macros.h"
#include "lib/instances.h"

using std::vector;

class WasmMemoryInstance;
struct WasmFuncInstance;
struct WasmGlobalInstance;
struct WasmTableInstance;

// for storing global instances, it consists of -
// the runtime representation of all instances of functions, tables, memories, and globals -
// that have been allocated during the life time of the abstract machine.
struct Store {
  SET_STRUCT_MOVE_ONLY(Store);
  vector<WasmMemoryInstance> memoryInsts;
  vector<WasmFuncInstance> functionInsts;
  vector<WasmGlobalInstance> globalInsts;
  vector<WasmTableInstance> tableInsts;
};

#endif  // STORE_H_
