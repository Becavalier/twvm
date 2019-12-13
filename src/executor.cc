// Copyright 2019 YHSPY. All rights reserved.
#include "src/executor.h"
#include "src/include/macros.h"
#include "src/opcode.h"
#include "src/decoder.h"
#include "src/utils.h"

const int Executor::execute(shared_ptr<WasmInstance> wasmIns) {
  (Printer::instance() << '\n').debug();
  (Printer::instance() << "- [EXECUTING PHASE] -\n").debug();

  pc = wasmIns->startPoint->pc;
  contextIndex = wasmIns->startPoint->index;

  /**
   * the structure of an opcode action:
   * |----------|----------------------|-------------------------|
   * | OpCode 1 | Invoker(uintptr_t) 8 | Immediates(var/fixed) n |
   * |----------|----------------------|-------------------------|
   */
  while (true) {
    if (!runningStatus) {
      // verify running reuslt by the state of final stack;
      return wasmIns->stack->checkStackState(wasmIns->startEntry);
    }
    
    // don't use "switch-case" based conditional selection, since -
    // it's overhead from low (Branch-Table -> Binary-Decision-Tree -> if-else) to high, -
    // but not efficient enough on average.
    uintptr_t handlerPtr;
    // skip the identifying byte;
    memcpy(&handlerPtr, pc->data() + (innerOffset += 2), ptrSize);
    innerOffset += (ptrSize - 1);
    // direct call;
    reinterpret_cast<handlerProto*>(handlerPtr)(wasmIns, this);
  }
  return 0;
}

const void Executor::crawler(
  const uint8_t* buf, size_t length, const function<bool(WasmOpcode, size_t)> &callback) {
  // skip every opcode and immediate;
  size_t offset = 0;
  while (offset != length) {
    const auto opcode = static_cast<WasmOpcode>(*(buf + offset++));
    // move pointer to the immediates;
    offset += ptrSize;
    offset += OpCode::calcOpCodeEntityLen(buf + offset, opcode);
    if (callback && callback(opcode, offset)) {
      return;
    }
  }
}
