// Copyright 2019 YHSPY. All rights reserved.
#ifndef MACROS_H_
#define MACROS_H_

#include <src/utilities.h>

#define DEBUG_OUT \
  Utilities::reportDebug
#define ERROR_OUT \
  Utilities::reportError

// 1. user-defined destructor will prevent compiler from calling explicit move-constructor;
#define SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete

#define SET_STRUCT_MOVE_ONLY(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName);

#endif  // MACROS_H_
