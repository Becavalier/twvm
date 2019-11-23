// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include "src/opcode.h"
#include "src/macros.h"

bool OpCode::doUnreachable() {
  // trap;
  ERROR_OUT("unreachable code!");
  return false;
}

bool OpCode::handle(shared_ptr<WasmInstance> wasmIns, WasmOpcode opcode) {
  // deal with locals first;
  // std::cout << (int) opcode << std::endl;
  switch (opcode) {
    case WasmOpcode::kOpcodeUnreachable: {
      return doUnreachable();
      break;
    }
    default:
      break;
  }
};