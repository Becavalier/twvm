// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_COMMON_MACROS_H_
#define LIB_COMMON_MACROS_H_

// user-defined destructor will prevent compiler from calling -
// explicit move-constructor.
#define SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete

#define SET_STRUCT_MOVE_ONLY(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName);

#if __has_include(<lib/type.h>)
#define ITERATE_WASM_RT_VAL_TYPE(V) \
  V(I32, ValueFrameTypes::kRTI32Value, int32_t) \
  V(U32, ValueFrameTypes::kRTU32Value, uint32_t) \
  V(I64, ValueFrameTypes::kRTI64Value, int64_t) \
  V(U64, ValueFrameTypes::kRTU64Value, uint64_t) \
  V(F32, ValueFrameTypes::kRTF32Value, float) \
  V(F64, ValueFrameTypes::kRTF64Value, double)
#endif

#endif  // LIB_COMMON_MACROS_H_
