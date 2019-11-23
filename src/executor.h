// Copyright 2019 YHSPY. All rights reserved.
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <memory>
#include <src/instantiator.h>

using std::shared_ptr;

// core execution logic;
class Executor {
 private:
  static size_t codeLen;

 public:
  static size_t currentSteps;
  static int execute(shared_ptr<WasmInstance>);
};

#endif  // EXECUTOR_H_
