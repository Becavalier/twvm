// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_WASM_H_
#define LIB_INSTANCES_INS_WASM_H_

#include <memory>
#include "lib/type.h"
#include "lib/common/macros.h"
#include "lib/instances/ins-module.h"
#include "lib/structures/struct-stack.h"
#include "lib/structures/struct-store.h"

using std::shared_ptr;

struct WasmInstance {
  SET_STRUCT_MOVE_ONLY(WasmInstance);
  // start function, or main function;
  shared_ptr<PosPtr> startPoint = nullptr;
  bool startEntry = true;
  shared_ptr<WasmModuleInstance> module;
  shared_ptr<Store> store;
  shared_ptr<Stack> stack;
};

#endif  // LIB_INSTANCES_INS_WASM_H_
