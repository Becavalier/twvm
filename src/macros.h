// Copyright 2019 YHSPY. All rights reserved.
#ifndef MACROS_H_
#define MACROS_H_

#define FORBID_COPYING(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete

#define MOVE_ONLY_STRUCT(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  FORBID_COPYING(TypeName);

#endif  // MACROS_H_
