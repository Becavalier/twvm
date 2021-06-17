// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INSTANTIATOR_H_
#define LIB_INSTANTIATOR_H_

#include <type_traits>
#include "lib/include/structs.hh"

namespace TWVM {
  class Instantiator {
    template<typename T>
    static void expandVTypesToRTValues(std::vector<Runtime::runtime_value_t>& container, T& t) {
      if constexpr (
        std::is_same_v<std::decay_t<T>, std::vector<uint8_t>>
      ) {
        for (const auto i : t) {
          switch (static_cast<ValueTypes>(i)) {
            case ValueTypes::I32: container.push_back(Runtime::rt_i32_t()); break;
            case ValueTypes::I64: container.push_back(Runtime::rt_i64_t()); break;
            case ValueTypes::F32: container.push_back(Runtime::rt_f32_t()); break;
            case ValueTypes::F64: container.push_back(Runtime::rt_f64_t()); break;
          }
        }
      }
    }
   public:
    static shared_module_runtime_t instantiate(shared_module_t);
    static Runtime::runtime_value_t evalInitExpr(uint8_t, std::vector<uint8_t>&);
    template<typename ...Args>
    static void expandWasmTypesToRTValues(
      std::vector<Runtime::runtime_value_t>& container, 
      Args& ...args) {
        (expandVTypesToRTValues(container, args), ...);
      }
  };
}

#endif  // LIB_INSTANTIATOR_H_
