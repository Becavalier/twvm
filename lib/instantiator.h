// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INSTANTIATOR_H_
#define LIB_INSTANTIATOR_H_

#include "lib/structs.h"

namespace TWVM {
  struct Instantiator {
    static const shared_module_instance_t instantiate(shared_module_t);
  };
}

#endif  // LIB_INSTANTIATOR_H_
