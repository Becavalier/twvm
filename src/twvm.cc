// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <string>
#include <chrono>
#include "src/utilities.h"
#include "src/constants.h"
#include "src/loader.h"
#include "src/module.h"
#include "src/executor.h"

using std::to_string;
using std::chrono::high_resolution_clock; 
using std::chrono::duration_cast;
using std::chrono::microseconds;

int main(int argc, char **argv) {
  const auto start = high_resolution_clock::now(); 

  if (argc < 2) {
    Utilities::reportError("no input file.");
    return 1;
  }

  const auto wasmModule = Loader::init(argv[INPUT_ARG_OFFSET]);
  // debug;
  if (wasmModule) {
    const auto wasmModuleSize = wasmModule->getModContentLength();
    if (wasmModuleSize > 0) {
      Utilities::reportDebug("module parsing completed. (" + to_string(wasmModuleSize) + " bytes)");
      const auto stop = high_resolution_clock::now();
      const auto duration = duration_cast<microseconds>(stop - start);
      Utilities::reportDebug() << "execution time: " << duration.count() / 1000.0 << "ms." << std::endl;
    }
  } else {
    return 1;
  }

  return 0;
}
