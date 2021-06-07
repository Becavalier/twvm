// Copyright 2021 YHSPY. All rights reserved.
#include <string>
#include <iostream>
#include "src/twvm.h"
#include "lib/loader.h"
#include "lib/instantiator.h"
#include "lib/executor.h"

int main(int argc, const char **argv) {
  const auto wasmModule = TWVM::Loader::load(argv[1]);
  const auto wasmInstance = TWVM::Instantiator::instantiate(wasmModule);
  TWVM::Executor::execute(wasmInstance);

  std::cout << 1;
  std::cout << 1;
  return 0;
}
