// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXECUTOR_H_
#define LIB_EXECUTOR_H_

#include <optional>
#include "lib/structs.h"

namespace TWVM {
  class Executor {
   public:
    uint8_t* pc;
    bool isRunning = true;
    Executor(uint8_t* pc) : pc(pc) {}
    static void execute(
      shared_module_instance_t, 
      std::optional<uint32_t> = std::optional<uint32_t>());
  };
}

#endif  // LIB_EXECUTOR_H_
