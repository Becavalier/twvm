// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_MODULE_H_
#define LIB_INSTANCES_INS_MODULE_H_

#include <vector>
#include "lib/structures/struct-module.h"
#include "lib/common/macros.h"
#include "lib/instances/ins-section-func.h"
#include "lib/instances/ins-section-export.h"
#include "lib/instances/ins-section-global.h"
#include "lib/instances/ins-section-memory.h"
#include "lib/instances/ins-section-table.h"

using std::vector;

struct WasmModuleInstance {
  SET_STRUCT_MOVE_ONLY(WasmModuleInstance);
  shared_module_t staticModuleRef = nullptr;
  vector<WasmFunctionSig*> types;
  vector<WasmFuncInstance*> funcs;
  vector<WasmTableInstance*> tables;
  vector<WasmMemoryInstance*> memories;
  vector<WasmGlobalInstance*> globals;
  vector<WasmExportInstance> exports;
};

#endif  // LIB_INSTANCES_INS_MODULE_H_
