// Copyright 2021 YHSPY. All rights reserved.
#include <string>
#include <iostream>
#include "src/twvm.h"
#include "lib/include/loader.hh"
#include "lib/include/instantiator.hh"
#include "lib/include/executor.hh"
#include "lib/include/options.hh"
#include "lib/include/state.hh"
#include "lib/include/util.hh"
#include "lib/include/exception.hh"

using namespace TWVM;

int main(int argc, const char **argv) {
  // Setting up options.
  Options options { 
    "TWVM - A tiny, lightweight and efficient WebAssembly virtual machine.",
    [](auto* o, auto& v) {
      if (v.size() > 0) {
        State::createItem("path", v.front());  // Only using one path for now.
      }
    }
  };
  options.add(
    "-v", 
    "Show version number and built date.", 
    Options::OptionTypes::EXCLUSIVE,
    [](auto* o, auto& v) {
      Util::printAssistantInfo(false);
    });
  options.add(
    "-h", 
    "Show help information.", 
    Options::OptionTypes::EXCLUSIVE,
    [](auto* o, auto& v) {
      Util::printAssistantInfo();
      o->printOptions();
    });
  options.parse(argc, argv);

  // Running engine.
  const auto& inputPath = State::retrieveItem("path");
  if (inputPath.has_value()) {
    Executor::execute(
      Instantiator::instantiate(
        Loader::load((*inputPath)->toStr())));
  } else {
    Exception::terminate(Exception::ErrorType::INVALID_INPUT_PATH);
  }
  return 0;
}
