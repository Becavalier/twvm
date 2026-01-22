// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_INTERPRETER_HH_
#define LIB_INCLUDE_INTERPRETER_HH_

#include <array>
#include <optional>
#include "lib/include/opcodes.hh"
#include "lib/include/structs.hh"

#define DECLARE_OPCODE_HANDLER_VALID(NAME) \
  static void do##NAME(Executor&, op_handler_info_t = std::nullopt);
#define DECLARE_OPCODE_HANDLER_INVALID(NAME)
#define DECLARE_OPCODE_HANDLER(NAME, OP, VALDITI) \
  DECLARE_OPCODE_HANDLER_##VALDITI(NAME)

namespace TWVM {

class Executor;  // forward declaration.
struct Interpreter {
  using op_handler_proto_t = void (*)(Executor&, std::optional<uint32_t>);
  using op_handler_info_t = std::optional<uint32_t>;

  // For fallback mode (when computed goto is not supported)
  static std::array<op_handler_proto_t, sizeof(uint8_t) * 1 << 8> opTokenHandlers;

  // Main execution engine with direct threading support
  static void executeDirectThreaded(Executor& executor);

  ITERATE_ALL_OPCODE(DECLARE_OPCODE_HANDLER)
};

}  // namespace TWVM

#endif  // LIB_INCLUDE_INTERPRETER_HH_
