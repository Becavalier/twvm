// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_OPTION_H_
#define LIB_OPTION_H_

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
      OptHandlerType cb;
      OptionTypes type;
    };
    std::string appDesc;
    std::map<const std::string, Command> commands;
    std::vector<std::string> inputPaths;
    PathHandlerType pathHandler = nullptr;
   public:
    Options(const std::string& appDesc, PathHandlerType cb) : appDesc(appDesc), pathHandler(cb) {}
    void add(const std::string& name, const std::string desc, OptionTypes type, OptHandlerType cb) {
      commands.emplace(std::make_pair(name, Command { desc, cb, type }));
    }
    void printOptions() {
      std::stringstream ss;
      ss << appDesc << "\n\n";
      for (const auto& [x, y] : commands) {
        ss << ' ' << std::left << std::setw(OPTS_ARG_SETW) << x << y.desc << '\n';
      }
      std::cout << ss.str() << std::endl;
    }
    void parse(int, const char*[]);
  };
}

#endif  // LIB_OPTION_H_
