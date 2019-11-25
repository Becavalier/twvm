// Copyright 2019 YHSPY. All rights reserved.
#ifndef STACK_H_
#define STACK_H_

#include <stack>
#include <memory>
#include <type_traits>
#include "src/constants.h"
#include "src/utilities.h"
#include "src/macros.h"
#include "src/types.h"

using std::stack;
using std::shared_ptr;
using std::is_same;

class ValueFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(ValueFrameTypes type) : bitPattern{} {}

#define DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS(name, localtype, ctype) \
  ValueFrame(ctype v) : type(localtype), bitPattern{} { \
    Utilities::writeUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern), v); \
  } \
  const ctype to##name() { \
    return Utilities::readUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern)); \
  } 
  ITERATE_WASM_VAL_TYPE(DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS)

  const bool operator==(const ValueFrame& other) {
    return type == other.type && !memcmp(bitPattern, other.bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

 private:
  ValueFrameTypes type;
  uint8_t bitPattern[WASM_VALUE_BIT_PATTERN_WIDTH];
};

class LabelFrame {
 public:
  SET_STRUCT_MOVE_ONLY(LabelFrame);
 private:
  // for "block", "loop" and "if";
  ValueTypesCode resultType;
};

class ActivationFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ActivationFrame);
};

// for saving "Values" / "Labels" / "Activations";
class Stack {
 public:
  const int checkStackState() {
    // TODO(Jason Yu) check the status of stack;
    return 0;
  }
  // in order to reduce the overhead from casting between parent and child types -
  // caused by "dynamic_cast" and "static_cast", we'd better store these three kinds of Frames -
  // separately.
  stack<ValueFrame> valueStack;
  stack<LabelFrame> labelStack;
  stack<ActivationFrame> activationStack;
};

#endif  // STACK_H_
