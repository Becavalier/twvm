// Copyright 2019 YHSPY. All rights reserved.
#ifndef INSTANTIATOR_H_
#define INSTANTIATOR_H_

#include <vector>
#include <memory>
#include "src/module.h"
#include "src/constants.h"
#include "src/types.h"
#include "src/utilities.h"
#include "src/macros.h"
#include "src/stack.h"
#include "src/opcode.h"

using std::vector;
using std::make_shared;
using std::shared_ptr;

struct Store;
struct WasmModuleInstance;

class WasmMemoryInstance {
 public:
  SET_STRUCT_MOVE_ONLY(WasmMemoryInstance);
  WasmMemoryInstance(uint32_t initMemSize = 1, uint32_t maxMemSize = 0) : maxMemSize(maxMemSize) {
    if (initMemSize > 0 && (maxMemSize == 0 || initMemSize <= maxMemSize)) {
      // allocate space (multiple of Wasm page);
      data = reinterpret_cast<uchar_t*>(malloc(initMemSize * WASM_PAGE_SIZE));
      currentMemSize = initMemSize;
    } else {
      Utilities::reportError("invalid memory allocation size.");
    }
  }
  ~WasmMemoryInstance() {
    data = nullptr;
  }

  // memory -> stack;
  template <typename T>
  T load(uint32_t offset, uint32_t align) {
    if (offset + sizeof(T) <= currentMemSize * DEFAULT_BYTE_LENGTH) {
      return *reinterpret_cast<T*>(data + offset);
    } else {
      Utilities::reportError("memory out of bound.");
      // unreachable;
      return false;
    }
  }

  // stack -> memory;
  template <typename T>
  void store(uint32_t offset, uint32_t align, T val) {
    // bound check;
    if (offset + sizeof(T) <= currentMemSize * DEFAULT_BYTE_LENGTH) {
      *(reinterpret_cast<T*>(data + offset)) = val;
    } else {
      Utilities::reportError("memory out of bound.");
    }
  }

  // return the current memory size in Wasm pages;
  const auto size() {
    return currentMemSize;
  }

  const auto& rawData() {
    return data;
  }

  // expand the maxmium capacity of the memory by Wasm pages;
  const size_t grow(const size_t &pages) {
    if (maxMemSize != 0 && currentMemSize + pages > maxMemSize) {
      return -1;
    } else {
      const auto previousMemSize = currentMemSize;
      currentMemSize = currentMemSize + pages;
      return previousMemSize;
    }
  }

 private:
  // 64k per pages;
  uint32_t maxMemSize = 0;
  uint32_t currentMemSize = 0;
  uchar_t* data;
  const WasmMemory *staticMemory;
};

struct WasmGlobalInstance {
  SET_STRUCT_MOVE_ONLY(WasmGlobalInstance);
  ValueTypesCode type;
  RTValue val;
  bool mutability;
};

struct WasmTableInstance {
  SET_STRUCT_MOVE_ONLY(WasmTableInstance);
  uint32_t maxTableSize;
  vector<WasmFunction*> funcIndices;
};

struct WasmFuncInstance {
  SET_STRUCT_MOVE_ONLY(WasmFuncInstance);
  ~WasmFuncInstance() { code = nullptr; }
  WasmFunctionSig* type;
  shared_ptr<WasmModuleInstance> module;
  const vector<WasmOpcode> *code;
};

struct WasmExportInstance {
  SET_STRUCT_MOVE_ONLY(WasmExportInstance);
  // TODO (Jason Yu) ;
};

struct WasmModuleInstance {
  SET_STRUCT_MOVE_ONLY(WasmModuleInstance);
  vector<WasmFunctionSig*> types;
  vector<WasmFuncInstance*> funcs;
  vector<WasmTableInstance*> tables;
  vector<WasmMemoryInstance*> memmories;
  vector<WasmGlobalInstance*> globals;
  WasmExportInstance *exports;
};

struct WasmInstance {
  shared_ptr<WasmModuleInstance> module;
  shared_ptr<Store> store;
  shared_ptr<Stack> stack;
};

// instantiation;
class Instantiator {
 public:
  static const shared_ptr<WasmInstance> instantiate(shared_module_t);
};

#endif  // INSTANTIATOR_H_
