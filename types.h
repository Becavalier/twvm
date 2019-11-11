// Copyright 2019 YHSPY. All rights reserved.
#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include "./macros.h"

using std::string;
using uchar_t = unsigned char;

constexpr uint32_t kWasmMagicWord = 0x6d736100;
constexpr uint32_t kWasmVersion = 0x01;
constexpr size_t kSpecMaxWasmMemoryPages = 65536;
constexpr uint8_t kWasmTrue = 1;
constexpr uint8_t kWasmFalse = 0;

// basic types;
enum class valueTypesCode : int8_t {
  kVoid = 0x40,  // block_type;
  kI32 = 0x7f,
  kI64 = 0x7e,
  kF32 = 0x7d,
  kF64 = 0x7c,
  kFunc = 0x60,
  kFuncRef = 0x70
};

// import/export;
enum class externalTypesCode : uint8_t {
  kExternalFunction = 0,
  kExternalTable = 1,
  kExternalMemory = 2,
  kExternalGlobal = 3,
  kExternalException = 4
};

// sections;
enum class sectionTypesCode : uint8_t {
  kUnknownSection = 0,
  kTypeSection = 1,
  kImportSection = 2,
  kFunctionSection = 3,
  kTableSection = 4,
  kMemorySection = 5,
  kGlobalSection = 6,
  kExportSection = 7,
  kStartSection = 8,
  kElementSection = 9,
  kCodeSection = 10,
  kDataSection = 11,
  kDataCountSection = 12,
  kExceptionSection = 13
};

class WasmFunctionSig {
 public:
  WasmFunctionSig(size_t paramsCount, size_t returnCount, const valueTypesCode *reps)
    : paramsCount(paramsCount), returnCount(returnCount), reps(reps) {}
  ~WasmFunctionSig() {
    delete reps;
  }
 private:
  size_t paramsCount;
  size_t returnCount;
  const valueTypesCode *reps;
};

// wasm indirect call table;
struct WasmFunction {
  ~WasmFunction() {
    code = nullptr;
  }
  WasmFunctionSig* sig;
  size_t funcIndex;
  size_t sigIndex;
  const uchar_t *code;
  size_t codeLen;
  bool imported;
  bool exported;
};

struct WasmTable {
  MOVE_ONLY_STRUCT(WasmTable);
  valueTypesCode type = valueTypesCode::kFuncRef;
  uint32_t initialSize = 0;
  uint32_t maximumSize = 0;
  bool hasMaximumSize = false;
  bool imported = false;
  bool exported = false;
};

struct WasmMemory {
  MOVE_ONLY_STRUCT(WasmMemory);
  uint32_t initialPages = 0;
  uint32_t maximumPages = 0;
  bool hasMaximumPages = false;
  bool imported = false;
  bool exported = false;
};

struct WasmGlobal {
  valueTypesCode type;
  bool mutability;
  void* init;  // initialization expr;
  bool imported;
  bool exported;
};

struct WasmExport {
  string name;
  externalTypesCode type;
  uint32_t index;
};

#endif  // TYPES_H_
