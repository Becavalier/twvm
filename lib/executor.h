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
    struct FrameOffset {
      Runtime::stack_frame_t* ptr;
      uint32_t offset;
    };
    uint8_t* pc;
    uint8_t* storedPC;   
    shared_module_runtime_t rtIns;
    EngineStatus status = EngineStatus::EXECUTING;
    std::optional<Runtime::relative_depth_t> brIfDepth;
    std::vector<std::vector<uint32_t>> frameBitmap = { {}, {}, {} };
    size_t labelAboveActivFrameCount = 0;
    void crawlOpCodes(size_t);
   public:
    Executor(uint8_t* pc, shared_module_runtime_t rtIns) : pc(pc), rtIns(rtIns) {}
    const auto getCurrentStatus() const { return status; }
    const auto stopEngine() { 
      status = EngineStatus::STOPPED;
      // Check return arity.
      const auto& entryFrameOffset = refTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
      const auto& entryFrame = std::get<Runtime::RTActivFrame>(*((*entryFrameOffset).ptr));
      const auto& returnArity = entryFrame.returnArity;
      if (returnArity->size() > 0) {
        const auto v = std::get<Runtime::RTValueFrame>(rtIns->stack.back()).value;
        std::visit([](auto&& arg){ std::cout << arg; }, v);
      }
    }
    auto getEngineData() { return rtIns; }
    std::optional<uint32_t> getTopFrameIdx(Runtime::STVariantIndex, uint32_t = 0);
    void setInFrameBitmap(Runtime::STVariantIndex type, uint32_t idx) {
      frameBitmap.at(Util::asInteger(type)).push_back(idx);
    }
    void eraseFromFrameBitmap(Runtime::STVariantIndex type, uint32_t n) {
      auto& v = frameBitmap.at(Util::asInteger(type));
      v.erase(v.end() - n, v.end());
    }
    std::optional<FrameOffset> refTopFrameByType(Runtime::STVariantIndex, uint32_t = 0);
    auto getLabelAboveActivFrameCount() { return labelAboveActivFrameCount; }
    // PC-related methods.
    auto getPC() { return pc; }
    void setPC(uint8_t* addr) { pc = addr; }
    auto movPC(size_t steps = 1) { pc += steps; return pc; }
    uint8_t* lookupLabelContFromPC();
    auto decodeByteFromPC() {
      return *reinterpret_cast<uint8_t*>(pc++);
    }
    template<typename T>
    T decodeVaruintFromPC() {
      return Decoder::decodeVaruint<T>(pc);
    }
    template<typename T>
    T decodeVarintFromPC() {
      return Decoder::decodeVarint<T>(pc);
    }
    template<typename T>
    T decodeFloatingPointFromPC() {
      static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "Invalid de-referenced type.");
      const auto fv = *reinterpret_cast<T*>(pc);
      movPC(sizeof(T));
      return fv;
    }
    // Stack-related methods.
    template<typename T>
    auto& retrieveFromStack(size_t pos = 0) {
      try {
        return std::get<T>(
          rtIns->stack.at(rtIns->stack.size() - 1 - pos));
      } catch(const std::exception& e) {
        Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
      }
    }
    template<typename T>
    void pushToStack(T&& frame) {
      rtIns->stack.emplace_back(std::forward<T>(frame));
      const auto updateIdx = rtIns->stack.size() - 1;
      if constexpr (std::is_same_v<T, Runtime::RTActivFrame>) {
        labelAboveActivFrameCount = 0;
        setInFrameBitmap(Runtime::STVariantIndex::ACTIVATION, updateIdx);
      }
      if constexpr (std::is_same_v<T, Runtime::RTLabelFrame>) {
        labelAboveActivFrameCount++;
        setInFrameBitmap(Runtime::STVariantIndex::LABEL, updateIdx);
      }
    }
    void eraseFromStack(uint32_t startIdx, uint32_t posToEnd) {
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
    void validateArity(const Module::type_seq_t& arity) {
      for (auto i = 0; i < arity.size(); ++i) {
        if (((rtIns->stack.rbegin() + i)->index() + arity.at(i)) != MAGIC_VAR_INDEX_PLUS_TYPE) {
          Exception::terminate(Exception::ErrorType::ARITY_TYPE_MISMATCH);
        }
      }
    }
    // "Feed two, throw up one".
    template<typename T, typename U>
    void opHelperFTTO(std::function<U(T, T)> handler) {
        try {
          auto& x = std::get<Runtime::RTValueFrame>(rtIns->stack.back());  // "c2".
          auto& y = std::get<Runtime::RTValueFrame>(rtIns->stack.at(rtIns->stack.size() - 2));  // "c1".
          auto ret = handler(std::get<T>(y.value), std::get<T>(x.value));
          rtIns->stack.pop_back();  // Keep "c1" on the stage.
          y.value = ret;
        } catch (const std::exception& e) {
          Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
        }
      }
    static void execute(shared_module_runtime_t, std::optional<uint32_t> = {});
  };
}

#endif  // LIB_EXECUTOR_H_
