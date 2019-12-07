// Copyright 2019 YHSPY. All rights reserved.
#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include "src/types.h"
#include "src/include/macros.h"

using std::vector;
using std::shared_ptr;
using std::move;

// pay attention to the bound check;
#define WRAP_SELECT_METHOD(name, key) \
  inline const auto name() { return &key; } \
  inline const auto name(uint32_t index) \
    { return (index >= 0 && index < key.size()) ? &key[index] : nullptr; }

class Module {
 public:
  Module() = default;
  uint32_t importedFuncCount = 0;
  uint32_t importedTableCount = 0;
  uint32_t startFuncIndex = 0;
  bool hasValidStartFunc = false;

  void setModContent(const vector<uchar_t> &content) {
    moduleData = move(content);
    moduleDataBuf = moduleData.data();
    contentLength = moduleData.size();
  }

  inline size_t getModContentLength() const {
    return contentLength;
  }

  inline const uchar_t* getCurrentOffsetBuf() {
    return (moduleDataBuf + p);
  }

  inline void increaseBufOffset(size_t step) {
    p += step;
  }

  inline bool hasEnd() const {
    return contentLength == p;
  }

  WRAP_SELECT_METHOD(getTable, tables)
  WRAP_SELECT_METHOD(getFunctionSig, funcSignatures)
  WRAP_SELECT_METHOD(getFunction, functions)
  WRAP_SELECT_METHOD(getExport, exportTable)
  WRAP_SELECT_METHOD(getImport, importTable)
  WRAP_SELECT_METHOD(getGlobal, globals)
  WRAP_SELECT_METHOD(getElement, elements)

  inline auto& getMemory() { return memory; }

 private:
  vector<uchar_t> moduleData;
  const uchar_t *moduleDataBuf;
  size_t contentLength;
  // start from the first section;
  size_t p = 8;
  // params, returns;
  vector<WasmFunctionSig> funcSignatures;
  // order: external imported | internal defined;
  vector<WasmFunction> functions;
  vector<WasmTable> tables;
  shared_ptr<WasmMemory> memory = nullptr;
  vector<WasmGlobal> globals;
  vector<WasmExport> exportTable;
  vector<WasmImport> importTable;
  vector<WasmElement<>> elements;
};

using shared_module_t = shared_ptr<Module>;

#endif  // MODULE_H_
