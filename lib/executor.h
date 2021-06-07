// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXECUTOR_H_
#define LIB_EXECUTOR_H_

#include "lib/structs.h"

namespace TWVM {
  class Executor {
   public:
    static void execute(shared_module_instance_t);
  };
}

#endif  // LIB_EXECUTOR_H_
