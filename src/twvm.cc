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
#include "lib/include/constants.hh"
#include "lib/include/exception.hh"

using namespace TWVM;

int main(int argc, const char **argv) {
  // Setting up options.
  Options options { 
    "TWVM - A tiny, lightweight and efficient WebAssembly virtual machine.\n\n> Usage: \n\n twvm <file> [<option>]*",
    [](auto* o, auto& v) {
      if (v.size() > 0) {
        State::createItem("path", v.front());  // Only using one path for now.
      }
    }
  };
  options.add(
    "-i", 
    "<anyfunc>[,<arg>]*",
    "Invoke an exported function with input argument(s).", 
    Options::OptionTypes::COMMON,
    [](auto* o, auto& v) {
      if (v.has_value()) {
        const auto& args = *v;
        const auto comma = args.find_first_of(',');
        State::createItem(INPUT_ENTRY_KEY_NAME, args.substr(0, comma));
        if (comma != std::string_view::npos) {
          State::createItem(INPUT_ENTRY_KEY_ARG, args.substr(comma + 1));
        }
      }
    });
  options.add(
    "-v",
    std::nullopt,
    "Show version number and built date.", 
    Options::OptionTypes::EXCLUSIVE,
    [](auto* o, auto& v) {
      Util::printAssistantInfo(false);
    });
  options.add(
    "-h", 
    std::nullopt,
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
    const auto ret = Executor::execute(
      Instantiator::instantiate(
        Loader::load((*inputPath)->toStr())));
    if (ret.has_value()) {
      std::visit([](auto&& arg){ std::cout << arg; }, *ret);
    }
  } else {
    Exception::terminate(Exception::ErrorType::INVALID_INPUT_PATH);
  }
  return 0;
}
