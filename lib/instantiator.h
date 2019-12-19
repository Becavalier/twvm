// Copyright 2019 YHSPY. All rights reserved.
#ifndef INSTANTIATOR_H_
#define INSTANTIATOR_H_

#include <memory>
#include "lib/module.h"
#include "lib/type.h"
#include "lib/stack.h"
#include "lib/instances.h"

using std::shared_ptr;
using std::make_shared;

struct WasmInstance;

// instantiation;
class Instantiator {
 public:
  static const shared_ptr<WasmInstance> instantiate(shared_module_t);
};

#endif  // INSTANTIATOR_H_
