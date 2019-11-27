// Copyright 2019 YHSPY. All rights reserved.
#include "src/executor.h"
#include "src/macros.h"
#include "src/opcode.h"
#include "src/decoder.h"
#include "src/utils.h"

const int Executor::execute(shared_ptr<WasmInstance> wasmIns) {
  Utils::debug();
  Utils::debug("- [EXECUTING PHASE] -");

  codeLen = wasmIns->startCodeLen;
  pc = wasmIns->startPoint - 1;

  while (true) {
    if (currentSteps == codeLen) {
      // verify running reuslt by the state of final stack;
      return wasmIns->stack->checkStackState(wasmIns->startEntry);
    }

    const WasmOpcode opcode = static_cast<WasmOpcode>(Decoder::readUint8(forward_()));
    OpCode::handle(wasmIns, opcode, this);
  }

  return 0;
}
