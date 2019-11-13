// Copyright 2019 YHSPY. All rights reserved.
#ifndef OPCODE_H_
#define OPCODE_H_

#define ITERATE_NUMERIC_OPCODE(V) \
  V(I32Const, 0x41) \
  V(I64Const, 0x42) \
  V(F32Const, 0x43) \
  V(F64Const, 0x44)

#define ITERATE_PARAMETRIC_OPCODE(V) \
  V(Drop, 0x1a) \
  V(Select, 0xab)

#define ITERATE_VARIABLE_OPCODE(V) \
  V(LocalGet, 0x20) \
  V(LocalSet, 0x21) \
  V(LocalTee, 0x22) \
  V(GlobalGet, 0x23) \
  V(GlobalSet, 0x24)

#define ITERATE_CONTROL_OPCODE(V) \
  V(Unreachable, 0x00) \
  V(Nop, 0x01) \
  V(Block, 0x02) \
  V(Loop, 0x03) \
  V(If, 0x04) \
  V(Else, 0x05) \
  V(End, 0x0b) \
  V(Br, 0x0c) \
  V(BrIf, 0x0d) \
  V(BrTable, 0x0e) \
  V(Return, 0x0f) \
  V(Call, 0x10) \
  V(CallIndirect, 0x11)

#define DECLARE_NAMED_ENUM(name, opcode) \
  kOpcode##name = opcode,

enum WasmOpcode {
  ITERATE_NUMERIC_OPCODE(DECLARE_NAMED_ENUM)
  ITERATE_PARAMETRIC_OPCODE(DECLARE_NAMED_ENUM)
  ITERATE_VARIABLE_OPCODE(DECLARE_NAMED_ENUM)
  ITERATE_CONTROL_OPCODE(DECLARE_NAMED_ENUM)
};

#endif  // OPCODE_H_
