// Copyright 2019 YHSPY. All rights reserved.
#ifndef STACK_H_
#define STACK_H_

#include <stack>
#include <memory>
#include <type_traits>
#include "src/macros.h"
#include "src/types.h"

using std::stack;
using std::shared_ptr;
using std::unique_ptr;
using std::is_same;

class BaseValueFrame {};

template <typename T>
class ValueFrame : public BaseValueFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(T value) {
    if (is_same<T, int32_t>::value) {
      data.i32 = value;
    } else if (is_same<T, int64_t>::value) {
      data.i64 = value;
    } else if (is_same<T, float>::value) {
      data.f32 = value;
    } else if (is_same<T, double>::value) {
      data.f64 = value;
    }
  }
  const T& value() {
    return *reinterpret_cast<T*>(&data);
  };
 private:
   RTValue data;
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
    return 1;
  };
  
 private:
  // in order to reduce the overhead from casting between parent and child types -
  // caused by "dynamic_cast" and "static_cast", we'd better store these three kinds of Frames separately.
  stack<unique_ptr<BaseValueFrame>> valueStack;
  stack<LabelFrame> labelStack;
  stack<ActivationFrame> activationStack;
};

#endif  // STACK_H_
