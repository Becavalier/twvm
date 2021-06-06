// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INSTANTIATOR_H_
#define LIB_INSTANTIATOR_H_

#include <type_traits>
#include "lib/structs.h"

namespace TWVM {
  struct Instantiator {
    static shared_module_instance_t instantiate(shared_module_t);
    static runtime_value_t evalInitExpr(uint8_t, std::vector<uint8_t>&);
  };
}

#endif  // LIB_INSTANTIATOR_H_
