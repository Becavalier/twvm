// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_OPTIONS_HH_
#define LIB_INCLUDE_OPTIONS_HH_

#include <vector>
#include <string>
#include <map>
#include <utility>
#include <string_view>
#include <optional>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "lib/include/constants.hh"

namespace TWVM {

struct Options {
  enum class OptionTypes : uint8_t {
    COMMON,
    EXCLUSIVE,
  };
 private:
  using opt_handler_t = void (*)(Options*, const std::optional<std::string>&);
  using path_handler_t = void (*)(Options*, const std::vector<std::string>&);
  struct Command {
    const std::string desc;
    const std::optional<const std::string> args;
    opt_handler_t cb;
    OptionTypes type;
  };
  std::string appDesc;
  std::map<const std::string, Command> commands;
  std::vector<std::string> inputPaths;
  path_handler_t pathHandler = nullptr;
 public:
  Options(const std::string& appDesc, path_handler_t cb) : appDesc(appDesc), pathHandler(cb) {}
  void add(
    const std::string& name,
    const std::optional<const std::string>& args,
    const std::string& desc,
    OptionTypes type,
    opt_handler_t cb) {
      commands.emplace(std::make_pair(name, Command { desc, args, cb, type }));
    }
  void printOptions();
  void parse(int, const char*[]);
};

}  // namespace TWVM

#endif  // LIB_INCLUDE_OPTIONS_HH_
