// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <memory>
#include "src/opcode.h"
#include "src/macros.h"
#include "src/instantiator.h"

using std::shared_ptr;

class OpCode {
 private:
  int8_t inline doUnreachable() {
    // trap;
    ERROR_OUT("unreachable code!");
    return 1;
  }

 public:
  int8_t handle(shared_ptr<WasmInstance> wasmIns, WasmOpcode opcode) {
    switch (opcode) {
      case WasmOpcode::kOpcodeUnreachable: {
        return doUnreachable();
        break;
      }
      default:
        break;
    }
  };
};
