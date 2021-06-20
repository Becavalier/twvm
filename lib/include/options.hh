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
  using OptHandlerType = void (*)(Options*, const std::optional<std::string>&);
  using PathHandlerType = void (*)(Options*, const std::vector<std::string>&);
  struct Command {
    const std::string desc;
    const std::optional<const std::string> args;
    OptHandlerType cb;
    OptionTypes type;
  };
  std::string appDesc;
  std::map<const std::string, Command> commands;
  std::vector<std::string> inputPaths;
  PathHandlerType pathHandler = nullptr;
 public:
  Options(const std::string& appDesc, PathHandlerType cb) : appDesc(appDesc), pathHandler(cb) {}
  void add(
    const std::string& name,
    const std::optional<const std::string>& args,
    const std::string& desc,
    OptionTypes type,
    OptHandlerType cb) {
      commands.emplace(std::make_pair(name, Command { desc, args, cb, type }));
    }
  void printOptions();
  void parse(int, const char*[]);
};

}  // namespace TWVM

#endif  // LIB_INCLUDE_OPTIONS_HH_
