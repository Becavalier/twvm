// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_OPCODES_H_
#define LIB_OPCODES_H_

namespace TWVM {
  enum class OpCodes : uint8_t {
    End = 0xb,
    I32Const = 0x41,
    I64Const = 0x42,
    F32Const = 0x43,
    F64Const = 0x44,
  };
}

#endif  // LIB_OPCODES_H_
