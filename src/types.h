// Copyright 2019 YHSPY. All rights reserved.
#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "src/macros.h"
#include "src/utilities.h"

using std::string;
using std::vector;
using uchar_t = unsigned char;

constexpr uint32_t kWasmMagicWord = 0x6d736100;
constexpr uint32_t kWasmVersion = 0x01;
constexpr size_t kSpecMaxWasmMemoryPages = 65536;
constexpr uint8_t kWasmTrue = 1;
constexpr uint8_t kWasmFalse = 0;

// basic types;
enum class ValueTypesCode : int8_t {
  kVoid = 0x40,  // block_type;
  kI32 = 0x7f,
  kI64 = 0x7e,
  kF32 = 0x7d,
  kF64 = 0x7c,
  kFunc = 0x60,
  kFuncRef = 0x70,
};

// import/export;
enum class ExternalTypesCode : uint8_t {
  kExternalFunction = 0,
  kExternalTable = 1,
  kExternalMemory = 2,
  kExternalGlobal = 3,
  kExternalException = 4,
};

// sections;
enum class SectionTypesCode : uint8_t {
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
  kExceptionSection = 13,
};

// stack frame types;
enum class StackFrameTypes : uint8_t {
  kValues = 0,
  kLabels,
  kActivations,
};

/******************/
/* compound types */
/******************/
struct WasmInitExpr {
#define WRAP_CONSTRUCTOR(type, kindEnum, key) \
  explicit WasmInitExpr(type v) : kind(kindEnum) { \
    val.key = v; \
  }

  enum WasmInitKind {
    kNone,
    kGlobalIndex,
    kRefFuncIndex,
    kI32Const,
    kI64Const,
    kF32Const,
    kF64Const,
    kRefNullConst,
  } kind = kNone;
  union {
    // constants;
    int32_t vI32Const;
    int64_t vI64Const;
    float vF32Const;
    double vF64Const;
    // op: get_global (refer to an immutable import);
    uint32_t vGlobalIndex;
    uint32_t vFuncIndex;
  } val;

  WasmInitExpr() = default;
  WRAP_CONSTRUCTOR(int32_t, kI32Const, vI32Const)
  WRAP_CONSTRUCTOR(int64_t, kI64Const, vI64Const)
  WRAP_CONSTRUCTOR(float, kF32Const, vF32Const)
  WRAP_CONSTRUCTOR(double, kF64Const, vF64Const)
  WasmInitExpr(WasmInitKind kind, uint32_t index) : kind(kind) {
    if (kind == kGlobalIndex) {
      val.vGlobalIndex = index;
    } else if (kind == kRefFuncIndex) {
      val.vFuncIndex = index;
    } else {
      Utilities::reportError("unknown global type.", true);
    }
  }
};

/******************/
/* sections types */
/******************/
struct WasmFunctionSig {
  ~WasmFunctionSig() { reps = nullptr; }
  size_t paramsCount;
  size_t returnCount;
  const ValueTypesCode *reps;
};

// wasm indirect call table;
struct WasmFunction {
  ~WasmFunction() { code = nullptr; }
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
  ValueTypesCode type = ValueTypesCode::kFuncRef;
  uint32_t initialSize = 0;
  uint32_t maximumSize = 0;
  bool hasMaximumSize = false;
  bool imported = false;
  bool exported = false;
};

template <typename T = WasmFunction>
struct WasmElement {
  MOVE_ONLY_STRUCT(WasmElement<T>);
  size_t tableIndex = 0;  // default;
  ExternalTypesCode type = ExternalTypesCode::kExternalException;  // anyfunc;
  WasmInitExpr init = {};  // offset in table (initialization expr);
  vector<T*> entities = {};
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
  MOVE_ONLY_STRUCT(WasmGlobal);
  ValueTypesCode type = ValueTypesCode::kVoid;
  bool mutability = false;
  WasmInitExpr init = {};  // initialization expr;
  bool imported = false;
  bool exported = false;
};

struct WasmExport {
  string name;
  ExternalTypesCode type;
  uint32_t index;
};

struct WasmImport {
  string moduleName;
  string fieldName;
  ExternalTypesCode kind;
  uint32_t index;
};


#endif  // TYPES_H_
