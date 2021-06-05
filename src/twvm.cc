// Copyright 2021 YHSPY. All rights reserved.
#include <string>
#include <iostream>
#include "src/twvm.h"


int main(int argc, const char **argv) {
  const auto wasmModule = TWVM::Loader::init(argv[1]);
  
  return 0;
}
