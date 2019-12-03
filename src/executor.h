// Copyright 2019 YHSPY. All rights reserved.
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <memory>
#include "src/opcode.h"
#include "src/cache.h"
#include "src/instances.h"

using std::shared_ptr;

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

  inline const uchar_t* absAddr() const {
    return pc->data() + innerOffset;
  }

  inline void switchStatus(bool flag) {
    runningStatus = flag;
  }

  inline const uchar_t* forward_(size_t step = 1) {
    innerOffset += step;
    return absAddr();
  }

  inline const auto& getCurrentOpcode() const {
    return currentOpcode;
  }
};

#endif  // EXECUTOR_H_
