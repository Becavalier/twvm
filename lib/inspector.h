// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSPECTOR_H_
#define LIB_INSPECTOR_H_

#include <memory>
#include "lib/instantiator.h"

using std::shared_ptr;

// validate the structure of "WasmInstance" before execution.
class Inspector {
 public:
  static void inspect(shared_ptr<WasmInstance>);
};

#endif  // LIB_INSPECTOR_H_
