// Copyright 2019 YHSPY. All rights reserved.
#include <array>
#include "src/executor.h"
#include "src/include/macros.h"
#include "src/include/constants.h"
#include "src/opcode.h"
#include "src/decoder.h"
#include "src/utility.h"

using std::array;

const bool Executor::execute(shared_ptr<WasmInstance> wasmIns) {
  (Printer::instance() << '\n').debug();
  (Printer::instance() << "- [EXECUTING PHASE] -\n").debug();

  pc = wasmIns->startPoint->pc;
  contextIndex = wasmIns->startPoint->index;

#if !defined(OPT_DCT)
  // build a handler lookup table;
  static array<handlerProto*, uint8Size * byteLen> opcodeTokenHandlers;
#define APPEND_HANDLER_TO_CONTAINER(name, opcode) \
  opcodeTokenHandlers[opcode] = OpCode::do##name;
  ITERATE_ALL_OPCODE(APPEND_HANDLER_TO_CONTAINER)
#endif

  while (true) {
    if (!runningStatus) {
      // verify running reuslt by the state of final stack;
      return wasmIns->stack->checkStackState(wasmIns->startEntry);
    }
#if defined(OPT_DCT)
    /**
     * the structure of an opcode action:
     * |----------|----------------------|-------------------------|
     * | OpCode 1 | Invoker(uintptr_t) 8 | Immediates(var/fixed) n |
     * |----------|----------------------|-------------------------|
     */
    // don't use "switch-case" based conditional selection, since -
    // it's overhead from low (Branch-Table -> Binary-Decision-Tree -> if-else) to high, -
    // but not efficient enough on average.
    uintptr_t handlerPtr;
    // skip the identifying byte;
    memcpy(&handlerPtr, pc->data() + (innerOffset += 2), ptrSize);
    innerOffset += (ptrSize - 1);
    // direct call;
    reinterpret_cast<handlerProto*>(handlerPtr)(wasmIns, this);
#else
    // TTC (use "[]" to avoid the overhead of bound-checking);
    opcodeTokenHandlers[(*pc)[++innerOffset]](wasmIns, this);
#endif
  }
  return false;
}

const void Executor::crawler(
  const uint8_t* buf, size_t length, const function<bool(WasmOpcode, size_t)> &callback) {
  // skip every opcode and immediate;
  size_t offset = 0;
  while (offset != length) {
    const auto opcode = static_cast<WasmOpcode>(*(buf + offset++));
    // move pointer to the immediates;
#if defined(OPT_DCT)
    offset += ptrSize;
#endif
    offset += OpCode::calcOpCodeEntityLen(buf + offset, opcode);
    if (callback && callback(opcode, offset)) {
      return;
    }
  }
}
