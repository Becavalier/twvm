// Copyright 2019 YHSPY. All rights reserved.
#include <algorithm>
#include "src/instantiator.h"
#include "src/store.h"
#include "src/decoder.h"

using std::find_if;
using std::hex;
using std::showbase;

const shared_ptr<WasmInstance> Instantiator::instantiate(shared_module_t module) {
  DEBUG_OUT() << endl;
  DEBUG_OUT("- [INSTANTIATING PHASE] -");

  // produce store, stack and module instance;
  DEBUG_OUT("instantiating store, stack and module instances.");
  const auto store = make_shared<Store>();
  const auto stack = make_shared<Stack>();
  const auto moduleInst = make_shared<WasmModuleInstance>();
  const auto staticFuncTypes = module->getFunctionSig();
  for (auto &i : (*staticFuncTypes)) {
    moduleInst->types.push_back(&i);
  }

  // memory instance;
  DEBUG_OUT("store: creating memory instances.");
  const auto staticMemory = module->getMemory();
  // we can not use "push_back" here, -
  // since the destructor will be called when the temp value is copied by default copy-constructor -
  // (even for move-constructor, we didn't use std::move), and the memory we allocated will be lost.
  // so, only allow the way of "placement-new" here.
  store->memoryInsts.emplace_back(staticMemory->initialPages, staticMemory->maximumPages);
  moduleInst->memories.push_back(&store->memoryInsts.back());
  // TODO(Jason Yu): init data section;

  // function instances;
  DEBUG_OUT("store: creating function instances.");
  const auto staticFunctions = module->getFunction();
  for (auto &i : *staticFunctions) {
    store->functionInsts.emplace_back();
    const auto ins = &store->functionInsts.back();
    for (auto j = 0; j < i.codeLen; j++) {
      ins->code.push_back(static_cast<WasmOpcode>(Decoder::readUint8(i.code + j)));
    }
    ins->type = i.sig;
    ins->module = moduleInst;
    ins->staticProto = &i;
  }
  // We need to perform this loop separately, since -
  // the address of the vector elements are not stable due to the "resize" of each "*_back";
  for (auto &i : store->functionInsts) {
    moduleInst->funcs.push_back(&i);
  }

  // global instances;
  DEBUG_OUT("store: creating global instances.");
  const auto staticGlobal = module->getGlobal();
  for (auto &i : *staticGlobal) {
    // skip platform-hosting imported global;
    if (i.type != ValueTypesCode::kFunc) {
      store->globalInsts.push_back({i.type, i.init.toRTValue(), i.mutability});
    }
  }
  for (auto &i : store->globalInsts) {
    moduleInst->globals.push_back(&i);
  }

  // table instances;
  DEBUG_OUT("store: creating table instances.");
  const auto staticTable = module->getTable();
  for (auto &i : *staticTable) {
    store->tableInsts.push_back({i.maximumSize});
    const auto tableInst = &store->tableInsts.back();
    for (auto &j : *module->getElement()) {
      // MVP: use default element section;
      if (j.tableIndex == 0) {
        for (const auto p : j.entities) {
          tableInst->funcIndices.push_back(p);
        }
      }
    }
  }
  for (auto &i : store->tableInsts) {
    moduleInst->tables.push_back(&i);
  }

  // export instances;
  DEBUG_OUT("store: creating export instances.");
  const auto staticExport = module->getExport();
  for (auto &i : *staticExport) {
    moduleInst->exports.push_back({i.name, i.type, i.index});
  }

  // global Wasm instance;
  const auto wasmIns = make_shared<WasmInstance>();
  wasmIns->module = moduleInst;
  wasmIns->store = store;
  wasmIns->stack = stack;

  // setup start point;
  const auto startFunctionIndex = module->getStartFuncIndex();
  bool hasStartPoint = true;
  if (startFunctionIndex != -1) {
    const auto &wasmFunc = module->getFunction()->at(startFunctionIndex);
    wasmIns->startPoint = wasmFunc.code;
    wasmIns->startCodeLen = wasmFunc.codeLen;
  } else {
    const auto exportItB = module->getExport()->begin();
    const auto exportItE = module->getExport()->end();
    const auto it = find_if(exportItB, exportItE, [](const WasmExport &exportEntity) -> auto {
      return exportEntity.name == "main"
        && exportEntity.type == ExternalTypesCode::kExternalFunction;
    });
    if (it != exportItE) {
      const auto &wasmFunc = module->getFunction()->at(it->index);
      wasmIns->startPoint = wasmFunc.code;
      wasmIns->startCodeLen = wasmFunc.codeLen;
    } else {
      hasStartPoint = false;
    }
  }
  if (hasStartPoint && wasmIns->startPoint != nullptr) {
    DEBUG_OUT() << "execution start point: " << hex << showbase
      << reinterpret_cast<uintptr_t>(wasmIns->startPoint)
      << '.' << endl;
  } else {
    ERROR_OUT("no start point found!");
  }

  return wasmIns;
}
