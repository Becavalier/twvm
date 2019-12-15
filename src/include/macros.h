// Copyright 2019 YHSPY. All rights reserved.
#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

// user-defined destructor will prevent compiler from calling -
// explicit move-constructor;
#define SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete

#define SET_STRUCT_MOVE_ONLY(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName);

#if __has_include(<src/types.h>)
#define ITERATE_WASM_VAL_TYPE(V) \
  V(I32, ValueFrameTypes::kI32Value, int32_t) \
  V(I64, ValueFrameTypes::kI64Value, int64_t) \
  V(F32, ValueFrameTypes::kF32Value, float) \
  V(F64, ValueFrameTypes::kF64Value, double)
#endif

#endif  // INCLUDE_MACROS_H_
