// Copyright 2019 YHSPY. All rights reserved.
#ifndef INSPECTOR_H_
#define INSPECTOR_H_

#include <memory>
#include "src/instantiator.h"

using std::shared_ptr;

// validate the structure of "WasmInstance" before execution;
class Inspector {
 public:
  static bool inspect(shared_ptr<WasmInstance>);
};

#endif  // INSPECTOR_H_
