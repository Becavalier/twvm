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
using std::static_pointer_cast;
using std::enable_shared_from_this;
using std::is_same;

class Frame : public enable_shared_from_this<Frame> {
 public:
  explicit Frame(StackFrameTypes type) : type(type) {}
  ~Frame() = default;

  template<typename T>
  shared_ptr<T> as() {
    return static_pointer_cast<T>(shared_from_this());
  }
 private:
  StackFrameTypes type = StackFrameTypes::kValues;
};

template <typename T>
class ValuesFrame : public Frame {
 public:
  SET_STRUCT_DISABLE_COPY_CONSTUCT(ValuesFrame);
  ValuesFrame(T value) : Frame(StackFrameTypes::kValues) {
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

class LabelsFrame : public Frame {
 public:
  SET_STRUCT_DISABLE_COPY_CONSTUCT(LabelsFrame);
  LabelsFrame() : Frame(StackFrameTypes::kLabels) {}
 private:
  // for "block", "loop" and "if";
  ValueTypesCode resultType;
};

class ActivationsFrame : public Frame {
 public:
  SET_STRUCT_DISABLE_COPY_CONSTUCT(ActivationsFrame);
  ActivationsFrame() : Frame(StackFrameTypes::kActivations) {}
};

// for saving "Values" / "Labels" / "Activations";
class Stack {
 public:
  Stack() = default;
  ~Stack() = default;
  
 private:
  stack<Frame*> data;
};

#endif  // STACK_H_
