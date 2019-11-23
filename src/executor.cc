// Copyright 2019 YHSPY. All rights reserved.
#include "src/executor.h"
#include "src/macros.h"
#include "src/opcode.h"
#include "src/decoder.h"

size_t Executor::currentSteps = 0;
size_t Executor::codeLen = 0;

int Executor::execute(shared_ptr<WasmInstance> wasmIns) {
  DEBUG_OUT() << endl;
  DEBUG_OUT() << "- [EXECUTING PHASE] -" << endl;

  codeLen = wasmIns->startCodeLen;
  const auto *pc = wasmIns->startPoint;
  
  while(true) {
    if (currentSteps == codeLen) {
      return wasmIns->stack->checkStackState();
    }

    const WasmOpcode opcode = static_cast<WasmOpcode>(Decoder::readUint8(pc + (currentSteps++)));
    const auto result = OpCode::handle(wasmIns, opcode);
  }

  return 0;
}