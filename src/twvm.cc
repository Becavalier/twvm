// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include "src/macros.h"
#include "src/utilities.h"
#include "src/constants.h"
#include "src/loader.h"
#include "src/module.h"
#include "src/executor.h"
#include "src/stack.h"
#include "src/instantiator.h"
#include "src/inspector.h"

using std::to_string;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::make_unique;

const auto calcTimeInterval(decltype(high_resolution_clock::now()) &previous) {
  const auto stop = high_resolution_clock::now();
  const auto duration = duration_cast<microseconds>(stop - previous);
  previous = stop;
  return duration.count() / 1000.0;
}

int main(int argc, char **argv) {
  auto start = high_resolution_clock::now();

  if (argc < 2) {
    ERROR_OUT("no input file.");
    return 1;
  }

  // static loading;
  const auto wasmModule = Loader::init(argv[INPUT_ARG_OFFSET]);

  // debug;
  if (wasmModule) {
    const auto wasmModuleSize = wasmModule->getModContentLength();
    if (wasmModuleSize > 0) {
      DEBUG_OUT("module parsing completed. (" + to_string(wasmModuleSize) + " bytes)");
      DEBUG_OUT()
        << "static parsing time: "
        << calcTimeInterval(start) << "ms."
         << std::endl;
    }
  } else {
    return 1;
  }

  // instantiating;
  const auto wasmInstance = Instantiator::instantiate(wasmModule);
  // debug;
  if (wasmInstance) {
    DEBUG_OUT("module instantiating completed.");
    DEBUG_OUT()
      << "instantiating time: "
      << calcTimeInterval(start) << "ms."
      << std::endl;
  } else {
    return 1;
  }

  // inspect;
  Inspector::inspect(wasmInstance);

  // execution
  const auto executor = make_unique<Executor>();
  const auto result = executor->execute(wasmInstance);

  if (!result) {
    ERROR_OUT("error occured while executing Wasm module!");
  }

  return result;
}
