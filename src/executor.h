// Copyright 2019 YHSPY. All rights reserved.
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include "src/types.h"
#include "src/frames.h"
#include "src/opcode.h"
#include "src/cache.h"
#include "src/instances.h"
#include "src/constants.h"

using std::shared_ptr;
using std::function;
using std::vector;
using std::unordered_map;

#define ITERATE_OPERANDS_VALUE_TYPES(V) \
  V(int32, int32_t) \
  V(int64, int64_t) \
  V(float, float) \
  V(double, double)

#define DECLARE_CONSTANT_POOL(name, type) \
  unordered_map<type, ValueFrame> name##ConstantPool = {};

#define DECLARE_CONSTANT_POOL_SETTERS(name, type) \
  inline auto checkUpConstant(type val) { \
    return &((name##ConstantPool.try_emplace(val, val).first->second)); \
  }

#define DECLARE_CONSTANT_POOL_DEBUGGER(name, type) \
  inline void name##ConstantPoolDebug(stringstream &ss) { \
    type counter = 0; \
    for (const auto& val : name##ConstantPool) { \
      val.second.outputValue(ss); \
      if (++counter != name##ConstantPool.size()) { \
        ss << ", "; \
      } \
    } \
  }

#define DECLARE_CACHE_OPERATIONS(name, type) \
  type name##UseImmesCache(function<void(size_t*, type*)> accessor) { \
    const auto opcodeStaticOffset = innerOffset; \
    const auto& result = cache->name##GetValueCache( \
      contextIndex, \
      opcodeStaticOffset); \
    type immediate = 0; \
    if (result.empty()) { \
      size_t step = 0; \
      accessor(&step, &immediate); \
      cache->name##SetValueCache(contextIndex, opcodeStaticOffset, immediate, step); \
    } else {\
      immediate = result[DEFAULT_ELEMENT_INDEX]; \
      innerOffset += (result[DEFAULT_ELEMENT_INDEX + 1]); \
    } \
    return immediate; \
  }

enum class WasmOpcode;
struct WasmInstance;

// core execution logic;
class Executor {
 private:
  bool runningStatus = true;
  WasmOpcode currentOpcode;
  ITERATE_OPERANDS_VALUE_TYPES(DECLARE_CONSTANT_POOL)

 public:
  vector<uint8_t> *pc;
  size_t innerOffset = -1;
  uint32_t contextIndex = -1;
  shared_ptr<Cache> cache = make_shared<Cache>();
  const int execute(shared_ptr<WasmInstance>);
  const void crawler(const uchar_t*, size_t, const function<bool(WasmOpcode, size_t)> &callback = nullptr);

  ITERATE_OPERANDS_VALUE_TYPES(DECLARE_CONSTANT_POOL_SETTERS)
  ITERATE_OPERANDS_VALUE_TYPES(DECLARE_CONSTANT_POOL_DEBUGGER)
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_OPERATIONS)

  int64_t int64UseMetaCache(OpcodeMeta type, function<void(int64_t*)> accessor) {
    const auto opcodeStaticOffset = innerOffset;
    const auto cacheVal = cache->int64GetMetaCache(contextIndex, opcodeStaticOffset, type);
    int64_t value = 0;
    if (cacheVal == 0) {
      accessor(&value);
      cache->int64SetMetaCache(contextIndex, opcodeStaticOffset, type, value);
    } else { value = cacheVal; }
    return value;
  }

  const auto& uint32UseMemargCache(function<void(uint32_t*, uint32_t*, size_t*)> accessor) {
    const auto opcodeStaticOffset = innerOffset;
    auto& argsVal = cache->uint32GetMemargCache(contextIndex, opcodeStaticOffset);
    if (argsVal.empty()) {
      size_t step = 0;
      uint32_t align = 0, offset = 0;
      accessor(&align, &offset, &step);
      // prevent from copy-constructing;
      cache->uint32SetMemargCache(contextIndex, opcodeStaticOffset, align, offset, step);
    }
    return argsVal;
  }

  inline const uchar_t* absAddr() {
    return pc->data() + innerOffset;
  }

  inline void switchStatus(bool flag) {
    runningStatus = flag;
  }

  inline const uchar_t* forward_(size_t step = 1) {
    innerOffset += step;
    return absAddr();
  }

  inline const auto getCurrentOpcode() const {
    return currentOpcode;
  }
};

#endif  // EXECUTOR_H_
