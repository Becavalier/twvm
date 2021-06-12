#include <cstdlib>
#include <cstring>
#include <variant>
#include <optional>
#include <algorithm>
#include "lib/instantiator.h"
#include "lib/opcodes.h"
#include "lib/util.h"
#include "lib/exception.h"
#include "lib/constants.h"
#include "lib/decoder.h"

namespace TWVM {
  Runtime::runtime_value_t Instantiator::evalInitExpr(uint8_t valType, std::vector<uint8_t> &initExprOps) {
    // In the MVP, to keep things simple, only four constant operators and `get_local` available, -
    // and `get_local` can only refer to an imported global.
    const auto valTypeT = static_cast<ValueTypes>(valType);
    if (
      valTypeT >= ValueTypes::F64 && 
      valTypeT <= ValueTypes::I32 && 
      (valType + initExprOps.front()) == MAGIC_OPCODE_PLUS_TYPE
    ) {
      auto* startByte = initExprOps.data() + 1;
      switch (valTypeT) {
        case ValueTypes::I32: return Decoder::decodeVarint<Runtime::rt_i32_t>(startByte);  // varint32.
        case ValueTypes::I64: return Decoder::decodeVarint<Runtime::rt_i64_t>(startByte);  // varint64.
        case ValueTypes::F32: return *reinterpret_cast<Runtime::rt_f32_t*>(startByte);
        case ValueTypes::F64: return *reinterpret_cast<Runtime::rt_f64_t*>(startByte);
        default: return Runtime::rt_i32_t();
      }
    } else {
      Exception::terminate(Exception::ErrorType::INVALID_GLOBAL_SIG);
    }
  }
  shared_module_runtime_t Instantiator::instantiate(shared_module_t mod) {
    auto executableIns = std::make_shared<Runtime>(mod);
    /* imports - type / num */
    // TODO: after MVP.
    
    /* globals - init */
    for (auto &i : mod->globals) {
      executableIns->rtGlobals.push_back(
        evalInitExpr(i.globalType.valType, i.initOpCodes));
    }

    /* func */
    for (auto i = 0; i < mod->funcDefs.size(); ++i) {
      const auto idx = mod->funcTypesIndices.at(i);
      const auto& funcType = mod->funcTypes.at(idx);
      const auto codeEntry = mod->funcDefs.at(idx).body.data();
      // Wasm types -> RT types (value).
      executableIns->rtFuncDescriptor.emplace_back(&funcType, codeEntry);
      expandWasmTypesToRTValues(
        executableIns->rtFuncDescriptor.back().localsDefault,
        mod->funcDefs.at(i).locals,
        funcType.first
      );
    }

    /* mem */
    for (auto &i : mod->mems) {
      if (i.memType.maximum == 0 ||
        (i.memType.maximum > 0 && i.memType.initial <= i.memType.maximum)) {
        const auto size = i.memType.initial * WASM_PAGE_SIZE;
        executableIns->rtMems.emplace_back( 
          size, static_cast<uint8_t*>(std::calloc(size, sizeof(uint8_t)))
        );
      } else {
        Exception::terminate(Exception::ErrorType::MEM_EXCEED_MAX);
      }
    }

    /* data */
    for (auto &i : mod->data) {
      const auto offset = evalInitExpr(Util::asInteger(ValueTypes::I32), i.initOpCodes);
      if (i.memIdx >= executableIns->rtMems.size()) {
        Exception::terminate(Exception::ErrorType::MEM_ACCESS_OOB);
      }
      const auto& memStart = executableIns->rtMems.at(i.memIdx);
      if (std::get<int32_t>(offset) + i.dataBytes.size() < memStart.size) {
        std::memcpy(
          memStart.ptr + std::get<int32_t>(offset), 
          i.dataBytes.data(), 
          i.dataBytes.size());
      } else {
        Exception::terminate(Exception::ErrorType::MEM_DATA_EXCEED_MAX);
      }
    }

    /* table */
    for (auto &i : mod->tables) {
      if (i.tableType.maximum == 0 ||
        (i.tableType.maximum > 0 && i.tableType.initial <= i.tableType.maximum)) {
        executableIns->rtTables.emplace_back(
          std::vector<std::optional<uint32_t>>(i.tableType.initial, std::nullopt));
      } else {
        Exception::terminate(Exception::ErrorType::TBL_EXCEED_MAX);
      }
    }

    /* element */
    for (auto &i : mod->elements) {
      const auto offset = evalInitExpr(Util::asInteger(ValueTypes::I32), i.initOpCodes);
      if (i.tblIdx >= executableIns->rtTables.size()) {
        Exception::terminate(Exception::ErrorType::TBL_ACCESS_OOB);
      }
      auto& tblStart = executableIns->rtTables.at(i.tblIdx);
      int32_t offsetIdx = std::get<int32_t>(offset);
      if (i.funcIndices.size() + offsetIdx < tblStart.size()) {
        std::for_each(
          i.funcIndices.begin(), 
          i.funcIndices.end(), 
          [&](uint32_t funcIdx) {
            tblStart.at(offsetIdx++) = funcIdx;
          });
      } else {
        Exception::terminate(Exception::ErrorType::TBL_ELEM_EXCEED_MAX);
      }
    }

    /* start section */
    // TODO: after exectuion engine.

    /* entry point */
    const auto entryFunc = std::find_if(
      mod->exports.begin(), 
      mod->exports.end(), 
      [](auto& item) -> bool {
        return item.name == "main" && item.extKind == EXT_KIND_FUNC;
      });
    if (entryFunc != mod->exports.end()) {
      executableIns->rtEntryIdx = entryFunc->extIdx;
    }

    return executableIns;
  }
}