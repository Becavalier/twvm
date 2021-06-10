// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXECUTOR_H_
#define LIB_EXECUTOR_H_

#include <optional>
#include <exception>
#include <type_traits>
#include "lib/structs.h"
#include "lib/exception.h"
#include "lib/decoder.h"
#include "lib/util.h"

namespace TWVM {
  class Executor {
    enum class EngineStatus : uint8_t {
      EXECUTING,  
      CRAWLING,  // Crawling continuation (for labels).
      STOPPED,
    };
    uint8_t* pc;
    uint8_t* storedPC;   
    shared_module_runtime_t rtIns;
    EngineStatus status = EngineStatus::EXECUTING;
    void fastCrawling(size_t);
   public:
    std::vector<int32_t> topFrameIdxHolder { -1, -1, -1 }; 
    Executor(uint8_t* pc, shared_module_runtime_t rtIns) 
      : pc(pc), rtIns(rtIns) {}
    const auto currentStatus() const { return status; }
    auto refDynData() { return rtIns; }
    auto movPC(size_t steps = 1) { pc += steps; return pc; }
    auto setPC(uint8_t* addr) { pc = addr; }
    // PC related methods.
    uint8_t* lookupLabelContFromPC() {  // don't mess this process with interpreter.
      status = EngineStatus::CRAWLING;
      storedPC = pc;
      size_t pairingCount = 0;
      while (status == EngineStatus::CRAWLING) {
        fastCrawling(pairingCount);
      }
      const auto cont = pc;
      pc = storedPC;
      return cont;
    }
    uint8_t decodeByteFromPC() {
      return *reinterpret_cast<uint8_t*>(pc++);
    }
    template<typename T>
    T decodeVaruintFromPC() {
      return Decoder::decodeVaruint<T>(pc);
    }
    template<typename T>
    T decodeFloatingPointFromPC() {
      static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "Invalid de-referenced type.");
      const auto fv = *reinterpret_cast<T*>(pc);
      movPC(sizeof(T));
      return fv;
    }
    int32_t getTopFrameIdx(Runtime::StackTypeVariantIndex type) {
      return topFrameIdxHolder.at(Util::asInteger(type));
    }
    void updateTopFrameIdx(Runtime::StackTypeVariantIndex type, int32_t idx = -1) {
      topFrameIdxHolder.at(Util::asInteger(type)) = idx > 0 ? idx : stackHeight() - 1;
    }
    auto& refTopFrame(Runtime::StackTypeVariantIndex type) {
      return rtIns->stack.at(getTopFrameIdx(type));
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
    template<typename T>
    auto pushToStack(T&& frame) {
      rtIns->stack.emplace_back(std::forward<T>(frame));
    }
    size_t stackHeight() {
      return rtIns->stack.size();
    }
    void popFromStack() {
      rtIns->stack.pop_back();
    }
    void popFromStackIfMeetType(Runtime::StackTypeVariantIndex type) {
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
