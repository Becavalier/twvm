// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_EXECUTOR_H_
#define LIB_EXECUTOR_H_

#include <optional>
#include <exception>
#include <type_traits>
#include <variant>
#include <functional>
#include "lib/structs.h"
#include "lib/exception.h"
#include "lib/decoder.h"
#include "lib/util.h"
#include "lib/constants.h"

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
    std::optional<Runtime::relative_depth_t> brIfDepth;
    std::vector<std::vector<uint32_t>> frameBitmap = { {}, {}, {} };
    size_t labelAboveActivFrameCount = 0;
    struct FrameOffset {
      Runtime::stack_frame_t* ptr;
      uint32_t offset;
    };
    void fastCrawling(size_t);
   public:
    Executor(uint8_t* pc, shared_module_runtime_t rtIns) : pc(pc), rtIns(rtIns) {}
    const auto currentStatus() const { return status; }
    auto refDynData() { return rtIns; }
    auto movPC(size_t steps = 1) { pc += steps; return pc; }
    auto setPC(uint8_t* addr) { pc = addr; }
    // Misc.
    void setBrIfDepthCache(Runtime::relative_depth_t v) { brIfDepth = v; }
    void delBrIfDepthCache() { brIfDepth.reset(); }
    auto getBrIfDepthCacheOr(Runtime::relative_depth_t v) {
      return brIfDepth.value_or(v);
    }
    std::optional<uint32_t> getTopFrameIdx(Runtime::STVariantIndex type, uint32_t n = 0) {
      const auto& v = frameBitmap.at(Util::asInteger(type));
      return v.size() > n ? 
        std::make_optional(*(v.rbegin() + n)) : 
        std::nullopt;
    }
    void setFrameBitmap(Runtime::STVariantIndex type, uint32_t idx) {
      frameBitmap.at(Util::asInteger(type)).push_back(idx);
    }
    void eraseFromFrameBitmap(Runtime::STVariantIndex type, uint32_t n) {
      auto& v = frameBitmap.at(Util::asInteger(type));
      v.erase(v.end() - n, v.end());
    }
    std::optional<FrameOffset> refTopFrameByType(Runtime::STVariantIndex type, uint32_t n = 0) {
      const auto topIdx = getTopFrameIdx(type, n);
      return topIdx.has_value() ? 
        std::make_optional(
          FrameOffset {
            &rtIns->stack.at(*topIdx), *topIdx,
          }) : std::nullopt;
    }
    auto getLabelAboveActivFrameCount() { return labelAboveActivFrameCount; }
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
      const auto updateIdx = rtIns->stack.size() - 1;
      if constexpr (std::is_same_v<T, Runtime::RTActivFrame>) {
        labelAboveActivFrameCount = 0;
        setFrameBitmap(Runtime::STVariantIndex::ACTIVATION, updateIdx);
      }
      if constexpr (std::is_same_v<T, Runtime::RTLabelFrame>) {
        labelAboveActivFrameCount++;
        setFrameBitmap(Runtime::STVariantIndex::LABEL, updateIdx);
      }
    }
    size_t stackHeight() {
      return rtIns->stack.size();
    }
    void eraseStack(uint32_t startIdx, uint32_t posToEnd) {
      rtIns->stack.erase(rtIns->stack.begin() + startIdx, rtIns->stack.end() - posToEnd);
    }
    void popFromStack() { rtIns->stack.pop_back(); }
    template<typename T>
    auto popValFromStack() {
      try {
        const T v = std::get<T>(
          std::get<Runtime::RTValueFrame>(rtIns->stack.back()).value);
        rtIns->stack.pop_back();
        return v;
      } catch(const std::exception& e) {
        Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
      }
    }
    void popFromStackIfMeetType(Runtime::STVariantIndex type) {
      if (rtIns->stack.back().index() == Util::asInteger(type)) {
        popFromStack();
      }
    }
    void validateArity(const Module::type_seq_t& arity) {
      for (auto i = 0; i < arity.size(); ++i) {
        if (((rtIns->stack.rbegin() + i)->index() + arity.at(i)) != CONST_OP_PLUS_TYPE) {
          Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
        }
      }
    }
    // "Feed two, throw up one".
    template<typename T>
    void instHelperFTTO(std::function<Runtime::rt_i32_t(T, T)> handler) {
        try {
          auto& x = std::get<Runtime::RTValueFrame>(rtIns->stack.back());  // c2.
          auto& y = std::get<Runtime::RTValueFrame>(*(rtIns->stack.rbegin() + 1));  // c1.
          auto ret = handler(std::get<T>(x.value), std::get<T>(y.value));
          rtIns->stack.pop_back();  // keep "c1" on the stage.
          y.value = ret;
        } catch (const std::exception& e) {
          Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
        }
      }
    static void execute(
      shared_module_runtime_t, 
      std::optional<uint32_t> = {});
  };
}

#endif  // LIB_EXECUTOR_H_
