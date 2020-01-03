// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INTERPRETER_H_
#define LIB_INTERPRETER_H_

#define DECLARE_MEMBER_HANDLER(name, opcode) \
  static handlerProto do##name;

#include <memory>
#include <vector>
#include <functional>
#include "lib/frames.h"
#include "lib/utility.h"
#include "lib/decoder.h"
#include "lib/executor.h"
#include "lib/common/opcode.h"
#include "lib/instances/ins-wasm.h"

using ::std::shared_ptr;
using ::std::vector;
using ::std::function;
using shared_wasm_t = shared_ptr<WasmInstance>;
using handlerProto = void (shared_wasm_t&, Executor*);

class Interpreter {
 private:
  template <typename T>
  static void storeMemarg(
    shared_wasm_t &wasmIns,
    Executor *executor,
    const function<void(const int32_t, WasmMemoryInstance *const&, const T)>&);

  static void retrieveMemarg(
    shared_wasm_t &wasmIns,
    Executor *executor,
    const function<void(const int32_t, WasmMemoryInstance *const&)>&);

  template <typename T>
  static void retrieveDoubleRTVals(
    shared_wasm_t &wasmIns,
    Executor *executor,
    const function<void(const shared_ptr<Stack::ValueFrameStack>&, ValueFrame *const&, ValueFrame *const&)>&);
    
  template <typename T>
  static void retrieveSingleRTVal(
    shared_wasm_t &wasmIns,
    Executor *executor,
    const function<void(const shared_ptr<Stack::ValueFrameStack>&, ValueFrame *const&)>&);

 public:
  static uint32_t calcOpCodeEntityLen(const uint8_t* buf, WasmOpCode opcode) {
    #define OPCODE_CASE(name, ...) \
      case WasmOpCode::kOpcode##name:
    switch (opcode) {
      case WasmOpCode::kOpcodeF32Const: { return f32Size; }
      case WasmOpCode::kOpcodeF64Const: { return f64Size; }
      ITERATE_OPCODE_NAME_WITH_ONE_VAR_IMME(OPCODE_CASE) {
        return Decoder::calcPassBytes(buf);
      }
      // "memory_immediate";
      ITERATE_OPCODE_NAME_WITH_TWO_VAR_IMME(OPCODE_CASE) {
        return Decoder::calcPassBytes(buf, 2);
      }
      case WasmOpCode::kOpcodeBrTable: {
        //
        return 0;
        break;
      }
      default: {
        return 0;
      }
    }
    return 0;
  }
  ITERATE_ALL_OPCODE(DECLARE_MEMBER_HANDLER)
};

#endif  // LIB_INTERPRETER_H_
