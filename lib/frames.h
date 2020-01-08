// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_FRAMES_H_
#define LIB_FRAMES_H_

#include <cstring>
#include <vector>
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include "lib/common/constants.h"
#include "lib/common/errors.h"
#include "lib/common/macros.h"
#include "lib/instances/ins-section-func.h"
#include "lib/type.h"
#include "lib/utility.h"

using ::std::memcpy;
using ::std::memcmp;
using ::std::vector;
using ::std::shared_ptr;
using ::std::ostream;
using ::std::is_same;
using ::std::move;
using ::std::unordered_map;

class ValueFrame {
 public:
  bool initialized = false;
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(ValueFrameTypes runtimeType) : bitPattern{} {}
  ValueFrame(const ValueFrame *other) : runtimeType(other->runtimeType), isValueZero(other->isValueZero) {
    // copy "bitPattern";
    memcpy(bitPattern, other->bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
    initialized = true;
  }

#define DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS(name, localtype, ctype) \
  ValueFrame(ctype v) : runtimeType(localtype), bitPattern{} { \
    isValueZero = (v == static_cast<ctype>(0)); \
    genericType = runtimeTypeMapper[runtimeType]; \
    Utility::writeUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern), v); \
    initialized = true; \
  } \
  inline const ctype to##name() const { \
    return Utility::readUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern)); \
  }
  ITERATE_WASM_RT_VAL_TYPE(DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS)

  const bool operator==(const ValueFrame& other) const {
    return runtimeType == other.runtimeType &&
      !memcmp(bitPattern, other.bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

  inline ValueFrameTypes getRTValueType() const {
    return static_cast<ValueFrameTypes>(runtimeType);
  }

  template <typename T>
  inline void resetValue(T v) {
    if constexpr (is_same<T, int32_t>::value) {
      runtimeType = ValueFrameTypes::kRTI32Value;
    } else if constexpr (is_same<T, uint32_t>::value) {
      runtimeType = ValueFrameTypes::kRTU32Value;
    } else if constexpr (is_same<T, int64_t>::value) {
      runtimeType = ValueFrameTypes::kRTI64Value;
    } else if constexpr (is_same<T, uint64_t>::value) {
      runtimeType = ValueFrameTypes::kRTU64Value;
    } else if constexpr (is_same<T, float>::value) {
      runtimeType = ValueFrameTypes::kRTF32Value;
    } else if constexpr (is_same<T, double>::value) {
      runtimeType = ValueFrameTypes::kRTF64Value;
    } else {
      Printer::instance().error(Errors::MISC_INVALID_VALUEFRAME);
    }
    genericType = runtimeTypeMapper[runtimeType];
    isValueZero = (v == static_cast<T>(0));
    Utility::writeUnalignedValue<T>(reinterpret_cast<uintptr_t>(bitPattern), v);
  }

  void outputValue(ostream &out) const {
    switch (runtimeType) {
      case ValueFrameTypes::kRTF32Value: { out << toF32(); break; }
      case ValueFrameTypes::kRTF64Value: { out << toF64(); break; }
      case ValueFrameTypes::kRTI32Value: { out << toI32(); break; }
      case ValueFrameTypes::kRTU32Value: { out << toU32(); break; }
      case ValueFrameTypes::kRTI64Value: { out << toI64(); break; }
      case ValueFrameTypes::kRTU64Value: { out << toU64(); break; }
      default: break;
    }
  }

  inline const auto isZero() const {
    return isValueZero;
  }

  inline const auto getGenericType() const {
    return genericType;
  }

  template <typename T>
  inline const T resolveValue() {
    return Utility::readUnalignedValue<T>(reinterpret_cast<uintptr_t>(bitPattern));
  }

 private:
  ValueTypesCode genericType;
  ValueFrameTypes runtimeType;
  bool isValueZero;
  uint8_t bitPattern[WASM_VALUE_BIT_PATTERN_WIDTH];
  unordered_map<ValueFrameTypes, ValueTypesCode> runtimeTypeMapper = {
    { ValueFrameTypes::kRTI32Value, ValueTypesCode::kI32 },
    { ValueFrameTypes::kRTU32Value, ValueTypesCode::kI32 },
    { ValueFrameTypes::kRTI64Value, ValueTypesCode::kI64 },
    { ValueFrameTypes::kRTU64Value, ValueTypesCode::kI64 },
    { ValueFrameTypes::kRTF32Value, ValueTypesCode::kF32 },
    { ValueFrameTypes::kRTF64Value, ValueTypesCode::kF64 },
  };
};

class LabelFrame {
 public:
  SET_STRUCT_MOVE_ONLY(LabelFrame);
  shared_ptr<PosPtr> end;
  shared_ptr<PosPtr> branch;
  shared_ptr<PosPtr> start;

  LabelFrame(
    ValueTypesCode resultType,
    size_t valueStackHeight) : resultType(resultType), valueStackHeight(valueStackHeight) {}

  inline const auto getResultType() const { return resultType; }
  inline const auto getValueStackHeight() const { return valueStackHeight; }

 private:
  // for "block", "loop" and "if";
  ValueTypesCode resultType;
  // determine the # of returning arity;
  size_t valueStackHeight = 0;
};

class ActivationFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ActivationFrame);
  const WasmFuncInstance *pFuncIns = nullptr;
  vector<ValueFrame*> locals = {};
  shared_ptr<PosPtr> leaveEntry;

  ActivationFrame(
    const WasmFuncInstance *pFuncIns,
    size_t valueStackHeight,
    size_t labelStackHeight,
    shared_ptr<PosPtr> leaveEntry = nullptr,
    vector<ValueFrame*> inputLocals = {}) :
    pFuncIns(pFuncIns),
    leaveEntry(leaveEntry),
    valueStackHeight(valueStackHeight),
    labelStackHeight(labelStackHeight) {
      if (locals.size() != 0) {
        locals = move(inputLocals);
      }
    }

  inline const auto getValueStackHeight() const { return valueStackHeight; }
  inline const auto getLabelStackHeight() const { return labelStackHeight; }

 private:
  // determine the # of returning arity;
  size_t valueStackHeight = 0;
  // determine whether we reach the "end" of the function;
  size_t labelStackHeight = 0;
};

#endif  // LIB_FRAMES_H_
