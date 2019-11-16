// Copyright 2019 YHSPY. All rights reserved.
#include "src/instantiator.h"
#include "src/store.h"
#include "src/decoder.h"

const shared_ptr<WasmInstance> Instantiator::instantiate(shared_module_t module) {
  Utilities::reportDebug("- [INSTANTIATING PHASE] -");

  // produce store, stack and module instance;
  Utilities::reportDebug("instantiating store, stack and module instances.");
  const auto store = make_shared<Store>();
  const auto stack = make_shared<Stack>();
  const auto moduleInst = make_shared<WasmModuleInstance>();
  const auto staticFuncTypes = module->getFunctionSig();
  for (auto &i : (*staticFuncTypes)) {
    moduleInst->types.push_back(&i);
  }
  
  // memory instance;
  Utilities::reportDebug("store: creating memory instances.");
  const auto staticMemory = module->getMemory();
  store->memoryInsts.push_back({staticMemory->initialPages, staticMemory->maximumPages});
  moduleInst->memmories.push_back(&store->memoryInsts.back());
  // TODO(Jason Yu): init data section;

  // function instances;
  Utilities::reportDebug("store: creating function instances.");
  const auto staticFunctions = module->getFunction();
  for (auto &i : *staticFunctions) {
    const auto code = new vector<WasmOpcode>();
    for (auto j = 0; j < i.codeLen; j++) {
      const auto opcode = static_cast<WasmOpcode>(Decoder::readUint8(i.code + j));
      if (opcode != WasmOpcode::kOpcodeEnd) {
        code->push_back(opcode);
      }
    }
    store->functionInsts.push_back({i.sig, moduleInst, code});
    moduleInst->funcs.push_back(&store->functionInsts.back());
  }

  // global instances;
  Utilities::reportDebug("store: creating global instances.");
  const auto staticGlobal = module->getGlobal();
  for (auto &i : *staticGlobal) {
    // skip platform-hosting imported global;
    if (i.type != ValueTypesCode::kFunc) {
      store->globalInsts.push_back({i.type, i.init.toRTValue(), i.mutability});
      moduleInst->globals.push_back(&store->globalInsts.back());
    }
  }

  // table instances;
  Utilities::reportDebug("store: creating table instances.");
  const auto staticTable = module->getTable();
  for (auto &i : *staticTable) {
    store->tableInsts.push_back({i.maximumSize});
    moduleInst->tables.push_back(&store->tableInsts.back());
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

  // TODO(Jason Yu): global instances;
  Utilities::reportDebug("store: creating export instances. [skip]");
  const auto staticExport = module->getExport();

  return make_shared<WasmInstance>(WasmInstance({moduleInst, store, stack}));
}
