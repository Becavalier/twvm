// Copyright 2019 YHSPY. All rights reserved.
#include <cstdlib>
#include <thread>
#include <iostream>
#include <string>
#include <memory>
#include <exception>
#include "src/twvm.h"
#include "lib/config.h"
#include "lib/cmdline.h"
#include "lib/inspector.h"

using std::thread;
using std::string;
using std::make_unique;
using std::exit;
using std::exception;
using std::cerr;

int main(int argc, const char **argv) {
  // set up command line arguments;
  Options options("twvm", "TWVM - A tiny, lightweight and efficient WebAssembly virtual machine.");
  options.addPositional("<*.wasm>", Options::Arguments::One,
    [](Options *o, const string& argument) -> auto {
      Config::executeModulePath = argument;
    });
  options.add(
    "--version", "-v",
    "Show version and building info.", Options::Arguments::Zero,
    [](Options *o, const string& argument) -> auto {
      Utility::drawLogoGraphic(false);
      exit(EXIT_SUCCESS);
    });
  options.add(
    "--debug", "-d",
    "Show debug info while executing the module.", Options::Arguments::Zero,
    [](Options *o, const string& argument) -> auto {
      Config::isDebugMode = true;
    });
  options.parse(argc, argv);

  try {
    // start executing;
    if (Config::executeModulePath.length() == 0) {
      Printer::instance().error(Errors::CMD_NO_FILE);
    }

    // start a timer;
    auto start = high_resolution_clock::now();

    // static loading;
    const auto wasmModule = Loader::init(Config::executeModulePath);
    (Printer::instance()
      << "static parsing time: "
      << calcTimeInterval(start)
      << "ms.\n").debug();

    // instantiating;
    const auto wasmInstance = Instantiator::instantiate(wasmModule);
    (Printer::instance()
      << "instantiating time: "
      << calcTimeInterval(start)
      << "ms. \n").debug();

    // inspect;
    if (Config::isDebugMode) {
      Inspector::inspect(wasmInstance);
    }
    
    // execution
    thread execThread([&wasmInstance]() -> void {
      const auto executor = make_unique<Executor>();
      executor->execute(wasmInstance);
    });
    if (execThread.joinable()) { execThread.join(); }
    (Printer::instance()
      << "executing time: "
      << calcTimeInterval(start) 
      << "ms. \n").debug();
  } catch (const exception& e) {
    cerr << "TRACKING_ERROR_CODE: " << e.what() << endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
