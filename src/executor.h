// Copyright 2019 YHSPY. All rights reserved.
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <memory>
#include "src/opcode.h"
#include "src/instances.h"

using std::shared_ptr;

struct WasmInstance;
enum class WasmOpcode;

// core execution logic;
class Executor {
 private:
  bool runningStatus = true;
  size_t codeLen = 0;

 public:
  const uchar_t *pc;
  size_t innerOffset = 0;
  const int execute(shared_ptr<WasmInstance>);
  const void crawler(const uchar_t*, size_t, const function<void(WasmOpcode, size_t)> &callback = nullptr);

  inline void increaseCodeLen(size_t step) {
    codeLen += step;
  }

  inline void switchStatus(bool flag) {
    runningStatus = flag;
  }

  inline const uchar_t* forward_(size_t step = 1) {
    innerOffset += step;
    return pc + innerOffset;
  }
};

#endif  // EXECUTOR_H_
