// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_STRUCT_H_
#define LIB_STRUCT_H_

#include <memory>
#include <type_traits>

namespace TWVM {
  struct Module {
    bool hasValidHeader = false;
    size_t lastParsedSectionId = 0;
    uint32_t version = 1;
  };
  using shared_module_t = std::shared_ptr<Module>;
}

#endif  // LIB_STRUCT_H_
