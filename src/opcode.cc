// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include "src/opcode.h"
#include "src/macros.h"
#include "src/decoder.h"

void OpCode::doUnreachable() {
  // trap;
  ERROR_OUT("unreachable code!");
}

void OpCode::doI32Const(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  // push an i32 value onto the stack;
  auto value = static_cast<int32_t>(Decoder::readVarInt<int32_t>(executor->forward_()));
  wasmIns->stack->valueStack.push({value});
}

void OpCode::handle(shared_ptr<WasmInstance> wasmIns, WasmOpcode opcode, Executor *executor) {
  std::cout << (int) opcode << std::endl;
  switch (opcode) {
    case WasmOpcode::kOpcodeUnreachable: {
      doUnreachable();
      break;
    }
    case WasmOpcode::kOpcodeEnd: {
      cout << "end" << endl;
      break;
    }
    case WasmOpcode::kOpcodeI32Const: {
      doI32Const(wasmIns, executor);
      break;
    }
    default:
      break;
  }
}
