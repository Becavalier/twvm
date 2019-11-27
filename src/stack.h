// Copyright 2019 YHSPY. All rights reserved.
#ifndef STACK_H_
#define STACK_H_

#include <stack>
#include <memory>
#include <cstring>
#include <iostream>
#include <type_traits>
#include "src/constants.h"
#include "src/utils.h"
#include "src/macros.h"
#include "src/types.h"

using std::stack;
using std::shared_ptr;
using std::is_same;
using std::ostream;
using std::dec;
using std::memcmp;

class ValueFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(ValueFrameTypes type) : bitPattern{} {}

#define DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS(name, localtype, ctype) \
  ValueFrame(ctype v) : type(localtype), bitPattern{} { \
    Utils::writeUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern), v); \
  } \
  const ctype to##name() { \
    return Utils::readUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern)); \
  } 
  ITERATE_WASM_VAL_TYPE(DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS)

  const bool operator==(const ValueFrame& other) {
    return type == other.type && !memcmp(bitPattern, other.bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

  inline ValueTypesCode getValueType() {
    return static_cast<ValueTypesCode>(type);
  }

  void outputValue(ostream &out) {
    switch (type) {
      case ValueFrameTypes::kF32Value: { out << toF32(); break; }
      case ValueFrameTypes::kF64Value: { out << toF64(); break; }
      case ValueFrameTypes::kI32Value: { out << toI32(); break; }
      case ValueFrameTypes::kI64Value: { out << toI64(); break; }
      default: break;
    }
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
  const WasmFunction *pFuncIns = nullptr;

  ActivationFrame(const WasmFunction *pFuncIns, size_t valueStackHeight, size_t labelStackHeight) : 
    pFuncIns(pFuncIns), valueStackHeight(valueStackHeight), labelStackHeight(labelStackHeight) {};
  
  inline auto getValueStackHeight() { return valueStackHeight; }
  inline auto getLabelStackHeight() { return labelStackHeight; }
 private:
  // determine the # of returning arity;
  const size_t valueStackHeight = 0;
  // determine whether we reach the "end" of the function;
  const size_t labelStackHeight = 0;
};

// for saving "Values" / "Labels" / "Activations";
class Stack {
 public:
  const bool checkStackState(bool startEntry = true) {
    // check the status of stack;
    const auto leftValueSize = valueStack.size();
    Utils::say() << "(" << (startEntry ? "start" : "main") << "): ";
    if (leftValueSize == 1) {
      valueStack.top().outputValue(cout << dec);
      valueStack.pop();
    } else {
      cout << "(void)";
    }
    cout << endl;
    return leftValueSize <= 1;
  }

  // in order to reduce the overhead from casting between parent and child types -
  // caused by "dynamic_cast" and "static_cast", we'd better store these three kinds of Frames -
  // separately.
  stack<ValueFrame> valueStack;
  stack<LabelFrame> labelStack;
  stack<ActivationFrame> activationStack;
  // for temporary saving return arity;
  stack<ValueFrame> returnValueStack;
};

#endif  // STACK_H_
