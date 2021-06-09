// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXECUTOR_H_
#define LIB_EXECUTOR_H_

#include <optional>
#include <exception>
#include "lib/structs.h"
#include "lib/exception.h"
#include "lib/decoder.h"
#include "lib/util.h"

namespace TWVM {
  class Executor {
    enum class EngineStatus : uint8_t {
      RUNNING, STOPPED,
    };
    shared_module_runtime_t rtIns;
    uint8_t* pc;
    EngineStatus status = EngineStatus::RUNNING;
   public:
    Executor(uint8_t* pc, shared_module_runtime_t rtIns) 
      : pc(pc), rtIns(rtIns) {}
    const auto currentStatus() const { return status; }
    auto refEngineData() { return rtIns; }
    auto movPC(size_t steps = 1) { pc += steps; return pc; }
    auto setPC(uint8_t* addr) { pc = addr; }
    // PC related methods.
    template<typename T>
    T decodeVaruintFromPC() {
      return Decoder::decodeVaruint<T>(pc);
    }
    // Stack related methods.
    template<typename T>
    auto& retrieveFromStack(size_t pos = 0) {
      try {
        auto& item = *(rtIns->stack.rbegin() + pos);
        return std::get<T>(item);
      } catch(const std::exception& e) {
        Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
      }
    }
    void popFromStack() {
      rtIns->stack.pop_back();
    }
    void popFromStackIfMeetType(Runtime::StackTypes type) {
      if (rtIns->stack.back().index() == Util::asInteger(type)) {
        popFromStack();
      }
    }
    static void execute(
      shared_module_runtime_t, 
      std::optional<uint32_t> = std::optional<uint32_t>());
  };
}

#endif  // LIB_EXECUTOR_H_
