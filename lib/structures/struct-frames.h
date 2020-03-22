// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_STRUCTURES_STRUCT_FRAMES_H_
#define LIB_STRUCTURES_STRUCT_FRAMES_H_

#include <cstring>
#include <vector>
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include "lib/type.h"
#include "lib/utility.h"
#include "lib/common/constants.h"
#include "lib/common/errors.h"
#include "lib/common/macros.h"
#include "lib/instances/ins-section-func.h"

using std::memcpy;
using std::memcmp;
using std::vector;
using std::shared_ptr;
using std::ostream;
using std::is_same;
using std::move;
using std::unordered_map;

class ValueFrame {
 public:
  bool initialized = false;
  SET_STRUCT_MOVE_ONLY(ValueFrame);
  ValueFrame(ValueFrameTypes runtimeType) : bitPattern{} {}
  ValueFrame(const ValueFrame *other) : isValueZero(other->isValueZero), runtimeType(other->runtimeType) {
    // copy "bitPattern".
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
  const ctype to##name() const { \
    return Utility::readUnalignedValue<ctype>(reinterpret_cast<uintptr_t>(bitPattern)); \
  }
  ITERATE_WASM_RT_VAL_TYPE(DEFINE_VALUEFRAME_TYPE_SPECIFIC_METHODS)

  const bool operator==(const ValueFrame& other) const {
    return runtimeType == other.runtimeType &&
      !memcmp(bitPattern, other.bitPattern, WASM_VALUE_BIT_PATTERN_WIDTH);
  }

  ValueFrameTypes getRTValueType() const {
    return static_cast<ValueFrameTypes>(runtimeType);
  }

  template <typename T>
  void resetValue(T v) {
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

  void outputValue(ostream&) const;

  const auto isZero() const {
    return isValueZero;
  }

  const auto getGenericType() const {
    return genericType;
  }

  template <typename T>
  const T resolveValue() {
    return Utility::readUnalignedValue<T>(reinterpret_cast<uintptr_t>(bitPattern));
  }

 private:
  bool isValueZero;
  ValueTypesCode genericType;
  ValueFrameTypes runtimeType;
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
  // for "block", "loop" and "if".
  const ValueTypesCode resultType;
  // determine the # of returning arity.
  const size_t valueStackHeight = 0;

  LabelFrame(
    const ValueTypesCode resultType,
    const size_t valueStackHeight) : resultType(resultType), valueStackHeight(valueStackHeight) {}
};

class ActivationFrame {
 public:
  SET_STRUCT_MOVE_ONLY(ActivationFrame);
  const WasmFuncInstance *pFuncIns = nullptr;
  // the number of locals can not exceed the params number.
  vector<ValueFrame*> locals = {};
  shared_ptr<PosPtr> leaveEntry;
  
  // determine the # of returning arity.
  const size_t valueStackHeight = 0;
  // determine whether we reach the "end" of the function.
  const size_t labelStackHeight = 0;

  ActivationFrame(
    const WasmFuncInstance *pFuncIns,
    const size_t valueStackHeight,
    const size_t labelStackHeight,
    shared_ptr<PosPtr> leaveEntry = nullptr,
    vector<ValueFrame*> inputLocals = {}) :
    pFuncIns(pFuncIns),
    leaveEntry(leaveEntry),
    valueStackHeight(valueStackHeight),
    labelStackHeight(labelStackHeight) {
      if (inputLocals.size() != 0) {
        // the number of passed arguments may less than params?
        locals = move(inputLocals);
      } else {
        locals = {pFuncIns->type->paramsCount, nullptr};
      }
    }
};

#endif  // LIB_STRUCTURES_STRUCT_FRAMES_H_
