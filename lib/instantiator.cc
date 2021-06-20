// Copyright 2021 YHSPY. All rights reserved.
#include <cstdlib>
#include <cstring>
#include <variant>
#include <optional>
#include <algorithm>
#include "lib/include/instantiator.hh"
#include "lib/include/opcodes.hh"
#include "lib/include/util.hh"
#include "lib/include/exception.hh"
#include "lib/include/constants.hh"
#include "lib/include/decoder.hh"
#if __has_include(<lib/include/state.hh>)
#include <string_view>
#include <string>
#include "lib/include/state.hh"
#endif

namespace TWVM {

Runtime::runtime_value_t Instantiator::convertStrToRTVal(const std::string& str, uint8_t type) {
  switch (static_cast<ValueTypes>(type)) {
    case ValueTypes::I32: return static_cast<Runtime::rt_i32_t>(std::stoi(str));
    case ValueTypes::I64: return static_cast<Runtime::rt_i64_t>(std::stol(str));
    case ValueTypes::F32: return static_cast<Runtime::rt_f32_t>(std::stof(str));
    case ValueTypes::F64: return static_cast<Runtime::rt_f64_t>(std::stod(str));
    default: Exception::terminate(Exception::ErrorType::INVALID_VAL_TYPE);
  }
}
Runtime::runtime_value_t Instantiator::evalInitExpr(uint8_t valType, std::vector<uint8_t> &initExprOps) {
  // In the MVP, to keep things simple, only four constant operators and `get_local` available.
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
  // TODO(Jason Yu): after MVP.

  /* globals - init */
  for (auto &i : mod->globals) {
    executableIns->rtGlobals.push_back(
      evalInitExpr(i.globalType.valType, i.initOpCodes));
  }

  /* func */
  for (auto i = 0; i < mod->funcDefs.size(); ++i) {
    const auto typeIdx = mod->funcTypesIndices.at(i);
    const auto& funcType = mod->funcTypes.at(typeIdx);
    const auto codeEntry = mod->funcDefs.at(i).body.data();
    // Wasm types -> RT types (value).
    executableIns->rtFuncDescriptor.emplace_back(&funcType, codeEntry);
    expandWasmTypesToRTValues(
      executableIns->rtFuncDescriptor.back().localsDefault,
      funcType.first,
      mod->funcDefs.at(i).locals);
  }

  /* mem */
  for (auto &i : mod->mems) {
    const auto memType = i.memType;
    if (memType.maximum == 0 ||
      (memType.maximum > 0 && memType.initial <= memType.maximum)) {
      const auto size = memType.initial * WASM_PAGE_SIZE;
      executableIns->rtMems.emplace_back(
        memType.initial, static_cast<uint8_t*>(std::calloc(size, sizeof(uint8_t))), memType.maximum);
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
    if (std::get<int32_t>(offset) + i.dataBytes.size() < memStart.size * WASM_PAGE_SIZE) {
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
    if (i.funcIndices.size() + offsetIdx <= tblStart.size()) {
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
  // TODO(Jason Yu): after exectuion engine.

  /* entry point */
  const auto entryFunc = std::find_if(
    mod->exports.begin(),
    mod->exports.end(),
    [](auto& item) -> bool {
#if __has_include(<lib/include/state.hh>)
      const auto& inputFuncName = State::retrieveItem(INPUT_ENTRY_KEY_NAME);
      return item.name ==
        (inputFuncName.has_value() ? (*inputFuncName)->toStr() : "main") &&
        (item.extKind == EXT_KIND_FUNC);
#else
      return item.name == "main" && item.extKind == EXT_KIND_FUNC;
#endif
    });
  const auto funcIdx = entryFunc->extIdx;
  if (entryFunc != mod->exports.end()) {
    executableIns->rtEntryIdx = funcIdx;
  }
#if __has_include(<lib/include/state.hh>)
  // Setup user input args.
  const auto& inputArgs = State::retrieveItem(INPUT_ENTRY_KEY_ARG);
  const auto& inputFuncArgTypes = executableIns->rtFuncDescriptor.at(funcIdx).funcType->first;
  auto argView = inputArgs.has_value() ? std::string_view((*inputArgs)->toStr()) : std::string_view {};
  for (auto i = 0; i < inputFuncArgTypes.size(); ++i) {
    if (argView.empty()) {
      Exception::terminate(Exception::ErrorType::NOT_ENOUGH_INPUT_ARGS);
    } else {
      const auto comma = argView.find_first_of(',');
      if (comma != std::string_view::npos) {
        executableIns->stack.push_back(
          Runtime::RTValueFrame {
            convertStrToRTVal(std::string(argView.substr(0, comma)), inputFuncArgTypes[i])
          });
        argView.remove_prefix(comma + 1);
      } else {
        executableIns->stack.push_back(
          Runtime::RTValueFrame {
            convertStrToRTVal((*inputArgs)->toStr(), inputFuncArgTypes[i])
          });
      }
    }
  }
#endif
  return executableIns;
}

}  // namespace TWVM
