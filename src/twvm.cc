// Copyright 2021 YHSPY. All rights reserved.
#include <string>
#include <iostream>
#include "src/twvm.h"
#include "lib/loader.h"
#include "lib/instantiator.h"

int main(int argc, const char **argv) {
  const auto staticWasmModule = TWVM::Loader::load(argv[1]);
  const auto wasmInstance = TWVM::Instantiator::instantiate(staticWasmModule);
  std::cout << 1;
  std::cout << 1;
  return 0;
}
