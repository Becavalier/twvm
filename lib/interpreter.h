// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INTERPRETER_H_
#define LIB_INTERPRETER_H_

#include <array>
#include "lib/opcodes.h"
#include "lib/structs.h"

#define DECLARE_OPCODE_HANDLER_VALID(NAME) \
  static void do##NAME(Executor&, shared_module_instance_t);
#define DECLARE_OPCODE_HANDLER_INVALID(NAME)
#define DECLARE_OPCODE_HANDLER(NAME, OP, VALDITI) \
  DECLARE_OPCODE_HANDLER_##VALDITI(NAME)
  
namespace TWVM {
  class Executor;  // forward declaration.
  struct Interpreter {
    using opHandlerProto = void (*)(Executor&, shared_module_instance_t);
    static std::array<opHandlerProto, sizeof(uint8_t) * 1 << 8> opTokenHandlers;
    ITERATE_ALL_OPCODE(DECLARE_OPCODE_HANDLER)
  };
}

#endif  // LIB_INTERPRETER_H_
