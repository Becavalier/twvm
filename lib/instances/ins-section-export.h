// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_SECTION_EXPORT_H_
#define LIB_INSTANCES_INS_SECTION_EXPORT_H_

#include <string>
#include "lib/type.h"
#include "lib/common/macros.h"

using std::string;

struct WasmExportInstance {
  SET_STRUCT_MOVE_ONLY(WasmExportInstance);
  string name;
  ExternalTypesCode type;
  uint32_t index = 0;
};

#endif  // LIB_INSTANCES_INS_SECTION_EXPORT_H_
