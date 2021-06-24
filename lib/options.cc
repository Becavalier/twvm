// Copyright 2021 YHSPY. All rights reserved.
#include <cstdlib>
#include <string_view>
#include "lib/include/options.hh"

namespace TWVM {

void Options::parse(int argc, const char* argv[]) {
  for (size_t i = 1; i < argc; ++i) {
    const auto arg = argv[i];
    auto cmd = std::string_view(arg);
    if (cmd.at(0) == '-') {
      const auto equal = cmd.find_first_of('=');
      const auto _cb = [&](const std::string& key, const std::optional<const char*> val) {
        const auto& op = commands.find(key);
        if (op != commands.end()) {
          op->second.cb(this, val);
          if (op->second.type == OptionTypes::EXCLUSIVE) {
            std::exit(EXIT_SUCCESS);
          }
        }
      };
      if (equal != std::string_view::npos) {
        const auto key = cmd.substr(0, equal);
        const auto val = cmd.substr(equal + 1);
        _cb(std::string(key), std::make_optional(val.data()));
      } else {
        _cb(arg, std::nullopt);
      }
    } else {
      inputPaths.push_back(arg);
    }
  }
  pathHandler(this, inputPaths);
}
void Options::printOptions() {
  std::stringstream ss;
  ss << appDesc << "\n\n> Options:\n\n";
  for (const auto& [x, y] : commands) {
    auto commandName = x;
    if (y.args.has_value()) {
      commandName += ("=" + *y.args);
    }
    ss << ' ' << std::left << std::setw(SCREEN_ARG_SETW_OFFSET) << commandName << y.desc << '\n';
  }
  std::cout << ss.str() << std::endl;
}

}  // namespace TWVM
