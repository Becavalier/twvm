#include <cstdlib>
#include <cstring>
#include <variant>
#include <optional>
#include "lib/instantiator.h"
#include "lib/opcodes.h"
#include "lib/util.h"
#include "lib/exception.h"
#include "lib/constants.h"
#include "lib/decoder.h"

namespace TWVM {
  runtime_value_t Instantiator::evalInitExpr(uint8_t valType, std::vector<uint8_t> &initExprOps) {
    // In the MVP, to keep things simple, only four constant operators and `get_local` available, -
    // and `get_local` can only refer to an imported global.
    const auto valTypeT = static_cast<ValueTypes>(valType);
    if (
      valTypeT >= ValueTypes::F64 && 
      valTypeT <= ValueTypes::I32 && 
      (valType + initExprOps.front()) == CONST_OP_PLUS_TYPE
    ) {
      switch (valTypeT) {
        case ValueTypes::I32: return Decoder::decodeVarint<RTI32>(initExprOps.data() + 1);  // varint32.
        case ValueTypes::I64: return Decoder::decodeVarint<RTI64>(initExprOps.data() + 1);  // varint64.
        case ValueTypes::F32: return *reinterpret_cast<RTF32*>(initExprOps.data() + 1);
        case ValueTypes::F64: return *reinterpret_cast<RTF64*>(initExprOps.data() + 1);
        default: return RTI32();
      }
    } else {
      Exception::terminate(Exception::ErrorType::INVALID_GLOBAL_SIG);
    }
  }
  shared_module_instance_t Instantiator::instantiate(shared_module_t mod) {
    auto executableIns = std::make_shared<Instance>(mod);
    /* imports - type / num */
    /* globals - init */
    for (auto &i : mod->globals) {
      executableIns->runtimeGlobals.push_back(
        evalInitExpr(i.globalType.valType, i.initOpCodes));
    }

    /* mem */
    // TODO: add current memory size, bound check in data section init.
    for (auto &i : mod->mems) {
      if (i.memType.maximum == 0 ||
        (i.memType.maximum > 0 && i.memType.initial <= i.memType.maximum)) {
        executableIns->runtimeMemRefs.push_back(
          static_cast<uint8_t*>(std::calloc(i.memType.initial * WASM_PAGE_SIZE, sizeof(uint8_t)))
        );
      } else {
        Exception::terminate(Exception::ErrorType::MEM_EXCEED_MAX);
      }
    }

    /* data */
    for (auto &i : mod->data) {
      const auto offset = evalInitExpr(Util::asInteger(ValueTypes::I32), i.initOpCodes);
      if (i.memIdx >= executableIns->runtimeMemRefs.size()) {
        Exception::terminate(Exception::ErrorType::MEM_ACCESS_OOB);
      }
      const auto memStart = executableIns->runtimeMemRefs.at(i.memIdx);
      std::memcpy(
        memStart + std::get<int32_t>(offset), 
        i.dataBytes.data(), 
        i.dataBytes.size());
    }

    /* table */
    for (auto &i : mod->tables) {
      if (i.tableType.maximum == 0 ||
        (i.tableType.maximum > 0 && i.tableType.initial <= i.tableType.maximum)) {
        executableIns->runtimeTables.emplace_back(
          std::vector<std::optional<uint32_t>>(i.tableType.initial, std::nullopt));
      } else {
        Exception::terminate(Exception::ErrorType::TBL_EXCEED_MAX);
      }
    }

    /* element */
    for (auto &i : mod->elements) {
      const auto offset = evalInitExpr(Util::asInteger(ValueTypes::I32), i.initOpCodes);
      if (i.tblIdx >= executableIns->runtimeTables.size()) {
        Exception::terminate(Exception::ErrorType::TBL_ACCESS_OOB);
      }
      auto tblStart = executableIns->runtimeTables.at(i.tblIdx);
      int32_t offsetIdx = std::get<int32_t>(offset);
      if (i.funcIndices.size() + offsetIdx < tblStart.size()) {
        std::for_each(
          i.funcIndices.begin(), 
          i.funcIndices.end(), 
          [&](uint32_t funcIdx) {
            std::cout << +offsetIdx << std::endl;
            tblStart.at(offsetIdx++) = funcIdx;
          });
      } else {
        Exception::terminate(Exception::ErrorType::TBL_ELEM_EXCEED_MAX);
      }
    }

    /* start function */
    // 
    return executableIns;
  }
}