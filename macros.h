// Copyright 2019 YHSPY. All rights reserved.
#ifndef MACROS_H_
#define MACROS_H_

#define MOVE_ONLY_STRUCT(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete \

#endif  // MACROS_H_
