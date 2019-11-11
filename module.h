// Copyright 2019 YHSPY. All rights reserved.
#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <iostream>
#include <memory>
#include "./types.h"
#include "./util.h"

using std::vector;
using std::shared_ptr;

class Module {
 public:
  Module() = default;
  ~Module() {
    Util::reportDebug("module has been destructed.");
  }

  void setModContent(const vector<uchar_t> &content) {
    buf = content.data();
    contentLength = content.size();
  }

  inline size_t getModContentLength(void) {
    return contentLength;
  }

  inline const uchar_t* getCurrentOffsetBuf(void) {
    return (buf + p);
  }

  inline void increaseBufOffset(size_t step) {
    p += step;
  }

  inline bool hasEnd() {
    return contentLength == p;
  }

  inline auto& getTable() { return tables; }
  inline auto& getFunctionSig() { return funcSignatures; }
  inline auto& getFunction() { return functions; }
  inline auto& getMemory() { return memory; }
  inline auto& getExport() { return exportTable; }
  inline auto& getGlobal() { return globals; }
  inline auto& getImportedFuncCount() { return importedFuncCount; }

 private:
  const uchar_t *buf;
  size_t contentLength;
  // start from the first section;
  size_t p = 8;
  size_t importedFuncCount = 0;
  // params, returns;
  vector<WasmFunctionSig*> funcSignatures;
  // order: (external imported) -> (internal defined);
  vector<WasmFunction> functions;
  vector<WasmTable> tables;
  shared_ptr<WasmMemory> memory;
  vector<WasmGlobal> globals;
  vector<WasmExport> exportTable;
};

using shared_module_t = shared_ptr<Module>;

#endif  // MODULE_H_
