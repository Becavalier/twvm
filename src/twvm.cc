// Copyright 2019 YHSPY. All rights reserved.
#include <cstdlib>
#include <thread>
#include <iostream>
#include <string>
#include <memory>
#include "src/twvm.h"
#include "lib/cmdline.h"
#include "lib/inspector.h"

using std::thread;
using std::string;
using std::make_unique;
using std::exit;

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

  // start executing;
  if (CommandLine::executeModulePath.length() == 0) {
    Printer::instance().error(Errors::CMD_NO_FILE);
  }

  auto start = high_resolution_clock::now();

  // static loading;
  const auto wasmModule = Loader::init(CommandLine::executeModulePath);
  // debug;
  if (wasmModule) {
    (Printer::instance() << "static parsing time: " << calcTimeInterval(start) << "ms.\n").debug();
  } else {
    exit(EXIT_FAILURE);
  }

  // instantiating;
  const auto wasmInstance = Instantiator::instantiate(wasmModule);
  // debug;
  if (wasmInstance) {
    (Printer::instance() << "module instantiating completed.\n").debug();
    (Printer::instance() << "instantiating time: " << calcTimeInterval(start) << "ms. \n").debug();
  } else {
    exit(EXIT_FAILURE);
  }

  // inspect;
  if (CommandLine::isDebugMode) {
    Inspector::inspect(wasmInstance);
  }

  // execution
  thread execThread([&wasmInstance]() -> void {
    const auto executor = make_unique<Executor>();
    executor->execute(wasmInstance);

    auto f = Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        0, 0x61, 0x73, 0x6d, 0x1, 0, 0, 0, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x10, 0x1, 0x0e, 0, 0x41, 0, 0x41, 0x7f, 0x36, 0x2, 0, 0x41, 0,
        0x28, 0x2, 0, 0x0b,
      }));
  Inspector::inspect(f);
  executor->execute(f
    );

  });
  if (execThread.joinable()) { execThread.join(); }
  (Printer::instance() << "executing time: " << calcTimeInterval(start) << "ms. \n").debug();

  return EXIT_SUCCESS;
}
