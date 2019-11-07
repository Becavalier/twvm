#ifndef TWVM_TYPES_H
#define TWVM_TYPES_H

#include <cstdint>

using uchar_t = unsigned char;

constexpr uint32_t kWasmMagicWord = 0x6d736100;
constexpr uint32_t kWasmVersion = 0x01;

// basic types;
enum class valueTypesCode : uint8_t {
  kVoid = 0x40,
  kI32 = 0x7f,
  kI64 = 0x7e,
  kF32 = 0x7d,
  kF64 = 0x7c,
  kS128 = 0x7b,
  kFuncRef = 0x70, // block_type;
};

constexpr uint8_t kFuncType = 0x60;

// import/export;
enum class external_types_code : uint8_t {
  kExternalFunction = 0,
  kExternalTable = 1,
  kExternalMemory = 2,
  kExternalGlobal = 3
};

// sections;
enum class section_types_code : int8_t {
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

#endif
