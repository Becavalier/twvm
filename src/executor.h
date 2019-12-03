// Copyright 2019 YHSPY. All rights reserved.
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <memory>
#include "src/opcode.h"
#include "src/cache.h"
#include "src/instances.h"

using std::shared_ptr;

#define DECLARE_CACHE_OPERATIONS(name, type) \
  type name##SetCache (function<type(size_t*)> accessor) { \
    const auto opcodeStaticOffset = innerOffset; \
    const vector<type>& result = cache->name##GetValueCache( \
      contextIndex, \
      opcodeStaticOffset); \
    type immediate; \
    if (result.empty()) { \
      size_t step = 1; \
      immediate = accessor(&step); \
      cache->name##SetValueCache(contextIndex, opcodeStaticOffset, immediate, step); \
    } else {\
      immediate = result[0]; \
      innerOffset += (result[1] - 1); \
    } \
    return immediate; \
  }

struct WasmInstance;
enum class WasmOpcode;

// core execution logic;
class Executor {
 private:
  bool runningStatus = true;
  WasmOpcode currentOpcode;

 public:
  vector<uint8_t> *pc;
  size_t innerOffset = -1;
  uint32_t contextIndex = -1;
  shared_ptr<Cache> cache = make_shared<Cache>();
  const int execute(shared_ptr<WasmInstance>);
  const void crawler(const uchar_t*, size_t, const function<bool(WasmOpcode, size_t)> &callback = nullptr);

  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_OPERATIONS);

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
