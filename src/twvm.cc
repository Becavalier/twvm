// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <cstdlib>
#include <string>
#include <chrono>
#include <memory>
#include "src/macros.h"
#include "src/utils.h"
#include "src/constants.h"
#include "src/loader.h"
#include "src/module.h"
#include "src/executor.h"
#include "src/stack.h"
#include "src/instantiator.h"
#include "src/inspector.h"
#include "src/cmdline.h"

using std::to_string;
using std::string;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::make_unique;
using std::exit;

const auto calcTimeInterval(decltype(high_resolution_clock::now()) &previous) {
  const auto stop = high_resolution_clock::now();
  const auto duration = duration_cast<microseconds>(stop - previous);
  previous = stop;
  return duration.count() / 1000.0;
}

int main(int argc, const char **argv) {
  // set up command line arguments;
  Options options("twvm", "TWVM - A tiny, lightweight and efficient WebAssembly virtual machine.");
  options.addPositional("<*.wasm>", Options::Arguments::One,
    [](Options *o, const string& argument) -> auto {
      CommandLine::executeModulePath = argument;
    });
  options.add(
    "--debug", "-d",
    "Show debugging info while executing the module.", Options::Arguments::Zero,
    [](Options *o, const string& argument) -> auto {
      CommandLine::isDebugMode = true;
    });
  options.parse(argc, argv);

  auto &printer = Printer::instance();

  // start executing;
  try {
    if (CommandLine::executeModulePath.length() == 0) {
      (printer << "no input file.\n").error();
      exit(1);
    }

    auto start = high_resolution_clock::now();

    // static loading;
    const auto wasmModule = Loader::init(CommandLine::executeModulePath);
    // debug;
    if (wasmModule) {
      const auto wasmModuleSize = wasmModule->getModContentLength();
      if (wasmModuleSize > 0) {
        (printer << "module parsing completed. (" << wasmModuleSize << ") bytes\n").debug();
        (printer << "static parsing time: " << calcTimeInterval(start) << "ms.\n").debug();
      }
    } else {
      exit(1);
    }

    // instantiating;
    const auto wasmInstance = Instantiator::instantiate(wasmModule);
    // debug;
    if (wasmInstance) {
      (printer << "module instantiating completed.\n").debug();
      (printer << "instantiating time: " << calcTimeInterval(start) << "ms. \n").debug();
    } else {
      exit(1);
    }

    // inspect;
    Inspector::inspect(wasmInstance);

    // execution
    const auto executor = make_unique<Executor>();
    const auto result = executor->execute(wasmInstance);

    if (!result) {
      (printer << "error occured while executing Wasm module.\n").error();
    }

    return result;
  } catch(const std::exception& e) {
    exit(1);
  }

  return 0;
}
