// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include "src/cmdline.h"
#include "src/utils.h"
#include "src/include/constants.h"

using std::ostream;
using std::string;
using std::cerr;
using std::max;

// initial configurations;
bool CommandLine::isDebugMode = false;
string CommandLine::executeModulePath = string();

void printWrap(ostream &os, int leftPad, const string &content) {
  int len = content.size();
  int space = SCREEN_WIDTH - leftPad;
  string nextWord;
  string pad(leftPad, ' ');
  for (int i = 0; i <= len; i++) {
    if (i != len && content[i] != ' ' && content[i] != '\n') {
      nextWord += content[i];
    } else {
      if (static_cast<int>(nextWord.size()) > space) {
        os << '\n' << pad;
        space = SCREEN_WIDTH - leftPad;
      }
      os << nextWord;
      space -= nextWord.size() + 1;
      if (space > 0) {
        os << ' ';
      }
      nextWord.clear();
      if (content[i] == '\n') {
        os << '\n';
        space = SCREEN_WIDTH - leftPad;
      }
    }
  }
}

Options::Options(const string &command, const string &description) : positional(Arguments::Zero) {
  add("--help", "-h", "Show help messages and exit.", Arguments::Zero,
    [this, command, description](Options *o, const string&) {
      cerr << command;
      if (positional != Arguments::Zero) {
        cerr << ' ' << positionalName;
      }
      cerr << "\n\n";
      printWrap(cerr, 0, description);
      cerr << "\n\nOptions:\n";
      size_t optionWidth = 0;
      for (const auto &o : options) {
        optionWidth = max(optionWidth, o.longName.size() + o.shortName.size());
      }
      for (const auto &o : options) {
        bool longNShort = o.longName.size() != 0 && o.shortName.size() != 0;
        size_t pad = 1 + optionWidth - o.longName.size() - o.shortName.size();
        cerr << "  " << o.longName << (longNShort ? ',' : ' ') << o.shortName << string(pad, ' ');
        printWrap(cerr, optionWidth + 4, o.description);
        cerr << '\n';
      }
      cerr << '\n';
      exit(EXIT_SUCCESS);
    });
}

Options& Options::add(
  const string &longName,
  const string &shortName,
  const string &description,
  Arguments arguments,
  const actionT &action) {
    options.push_back({longName, shortName, description, arguments, action, 0});
    return *this;
}

Options& Options::addPositional(const string &name, Arguments arguments, const actionT &action) {
  positional = arguments;
  positionalName = name;
  positionalAction = action;
  return *this;
}

void Options::parse(int argc, const char* argv[]) {
  size_t positionalsSeen = 0;
  auto dashes = [](const string &s) {
    for (size_t i = 0;; i++) {
      if (s[i] != '-') {
        return i;
      }
    }
  };
  for (size_t i = 1, e = argc; i != e; i++) {
    string currentOption = argv[i];
    if (dashes(currentOption) == 0) {
      switch (positional) {
        case Arguments::Zero: {
          (Printer::instance() << "Unexpected positional argument \'" << currentOption << "\'.\n").error(false);
          exit(EXIT_FAILURE);
        }
        case Arguments::One:
        case Arguments::Optional: {
          if (positionalsSeen) {
            (Printer::instance() 
              << "Unexpected second positional argument \'"
              << currentOption << "\' for " << positionalName << '\n').error(false);
            exit(EXIT_FAILURE);
          }
        }
        case Arguments::N: {
          positionalAction(this, currentOption);
          ++positionalsSeen;
          break;
        }
      }
      continue;
    }

    string argument;
    auto equal = currentOption.find_first_of('=');
    if (equal != string::npos) {
      argument = currentOption.substr(equal + 1);
      currentOption = currentOption.substr(0, equal);
    }
    Option* option = nullptr;
    for (auto &o : options)
      if (o.longName == currentOption || o.shortName == currentOption) {
        option = &o;
      }
    if (!option) {
      (Printer::instance() 
              << "Unknown option \'" << currentOption << "\'\n").error(false);
      exit(EXIT_FAILURE);
    }
    switch (option->arguments) {
      case Arguments::Zero: {
        if (argument.size()) {
          (Printer::instance() 
            << "Unexpected argument \'" << argument
            << "\' for option \'" << currentOption << "\'.\n").error(false);
          exit(EXIT_FAILURE);
        }
        break;
      }
      case Arguments::One: {
        if (option->seen) {
          (Printer::instance() 
            << "Unexpected second argument \'" << argument
            << "\' for \'" << currentOption << "\'.\n").error(false);
          exit(EXIT_FAILURE);
        }
      }
      case Arguments::N: {
        if (!argument.size()) {
          if (i + 1 == e) {
            (Printer::instance() << "Couldn\'t find expected argument for \'" << currentOption << "\'.\n").error(false);
            exit(EXIT_FAILURE);
          }
          argument = argv[++i];
        }
        break;
      }
      case Arguments::Optional: {
        if (!argument.size()) {
          if (i + 1 != e) {
            argument = argv[++i];
          }
        }
        break;
      }
    }
    option->action(this, argument);
    ++option->seen;
  }
}
