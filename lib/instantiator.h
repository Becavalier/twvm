// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANTIATOR_H_
#define LIB_INSTANTIATOR_H_

#include <memory>
#include "lib/type.h"
#include "lib/instances/ins-wasm.h"
#include "lib/structures/struct-module.h"
#include "lib/structures/struct-stack.h"

using std::shared_ptr;

// instantiation;
class Instantiator {
 public:
  static const shared_ptr<WasmInstance> instantiate(shared_module_t);
};

#endif  // LIB_INSTANTIATOR_H_
