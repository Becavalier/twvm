// Copyright 2019 YHSPY. All rights reserved.
#ifndef LOADER_H_
#define LOADER_H_

#define WRAP_UINT_FIELD(keyName, type, module) \
  const auto keyName = Decoder::readVarUint<type>(module)
#define WRAP_UINT_FIELD_WITH_STEP(keyName, type, module, step) \
  const auto keyName = Decoder::readVarUint<type>(module, step)
#define WRAP_UINT_FIELD_(type, module) \
  Decoder::readVarUint<type>(module)
#define WRAP_INT_FIELD(keyName, type, module) \
  const auto keyName = Decoder::readVarInt<type>(module)

#include <string>
#include <vector>
#include <memory>
#include "src/types.h"
#include "src/module.h"
#include "src/decoder.h"
#include "src/opcode.h"
#include "src/utils.h"

using std::vector;
using std::string;
using std::shared_ptr;
using std::make_shared;

class Loader {
 private:
  static vector<uchar_t> buf;
  static bool validateMagicWord(const vector<uchar_t>&);
  static bool validateVersionWord(const vector<uchar_t>&);
  static bool validateWords(const vector<uchar_t>&);

  // analyzer invokers;
  static void parse(const shared_module_t&);

  // analyzer helpers;
  static void parseSection(const shared_module_t&);
  static void parseUnkownSection(uint8_t, const shared_module_t&);
  static void parseTypeSection(const shared_module_t&);
  static void parseImportSection(const shared_module_t&);
  static void parseFunctionSection(const shared_module_t&);
  static void parseTableSection(const shared_module_t&);
  static void parseMemorySection(const shared_module_t&);
  static void parseStartSection(const shared_module_t&);
  static void parseGlobalSection(const shared_module_t&);
  static void parseExportSection(const shared_module_t&);
  static void parseCodeSection(const shared_module_t&);
  static void parseElementSection(const shared_module_t&);
  static void parseDataSection(const shared_module_t&);
  static void skipKnownSection(uint8_t, const shared_module_t&);

  // feeding module pointer directly (due to MVP version);
  static void consumeMemoryParams(const shared_module_t& module) {
    WRAP_UINT_FIELD(memoryFlags, uint8_t, module);
    WRAP_UINT_FIELD(initialPages, uint32_t, module);
    auto memory = make_shared<WasmMemory>();
    memory->initialPages = initialPages;

    // (0 : no /1: has) maximum field;
    if (memoryFlags == kWasmTrue) {
      WRAP_UINT_FIELD(maximumPages, uint32_t, module);
      memory->maximumPages = maximumPages;
      memory->hasMaximumPages = true;
    }
    module->getMemory() = memory;
  }

  static void consumeTableParams(const shared_module_t& module, WasmTable *const table) {
    WRAP_UINT_FIELD(tableFlags, uint8_t, module);
    WRAP_UINT_FIELD(initialSize, uint32_t, module);
    table->initialSize = initialSize;
    if (tableFlags == kWasmTrue) {
      WRAP_UINT_FIELD(maximumSize, uint32_t, module);
      table->maximumSize = maximumSize;
      table->hasMaximumSize = true;
    }
  }

  static void consumeInitExpr(const shared_module_t& module, WasmInitExpr *const  expr) {
    const auto opcode = static_cast<WasmOpcode>(Decoder::readUint8(module));

    // MVP: i32.const / i64.const / f32.const / f64.const / get_global;
    switch (opcode) {
      case WasmOpcode::kOpcodeI32Const: {
        expr->kind = InitExprKind::kI32Const;
        expr->val.vI32Const = Decoder::readVarInt<int32_t>(module);
        break;
      }
      case WasmOpcode::kOpcodeI64Const: {
        expr->kind = InitExprKind::kI64Const;
        expr->val.vI64Const = Decoder::readVarInt<int64_t>(module);
        break;
      }
      case WasmOpcode::kOpcodeF32Const: {
        expr->kind = InitExprKind::kF32Const;
        expr->val.vF32Const = Decoder::readUint32(module);
        break;
      }
      case WasmOpcode::kOpcodeF64Const: {
        expr->kind = InitExprKind::kF64Const;
        expr->val.vF64Const = Decoder::readUint64(module);
        break;
      }
      case WasmOpcode::kOpcodeGlobalSet: {
        WRAP_UINT_FIELD(globalIndex, uint32_t, module);
        const auto moduleGlobal = module->getGlobal(globalIndex);
        if (moduleGlobal->mutability || !moduleGlobal->imported) {
          Utils::report("only immutable imported globals can be used in initializer expressions.");
        }
        expr->kind = InitExprKind::kGlobalIndex;
        expr->val.vGlobalIndex = globalIndex;
        break;
      }
      default: {
        Utils::report("not supported opcode found in global section.");
        break;
      }
    }
    // "0x0b" ending byte;
    if (static_cast<WasmOpcode>(Decoder::readUint8(module)) != WasmOpcode::kOpcodeEnd) {
      Utils::report("illegal ending byte.");
    }
  }

 public:
  static shared_module_t init(const string&);
  static shared_module_t init(const uchar_t*, size_t);
};

#endif  // LOADER_H_
