// Copyright 2019 YHSPY. All rights reserved.
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include "src/cmdline.h"

#define INTERNAL_DEBUG_PREFIX_OUTPUT() \
  cout << OUTPUT_PREFIX << COLOR_CTL_DEBUG << "info: " << COLOR_CTL_NORMAL
#define INTERNAL_WARNING_PREFIX_OUTPUT() \
  cout << OUTPUT_PREFIX << COLOR_CTL_WARNING << "warning: " << COLOR_CTL_NORMAL
#define INTERNAL_ERROR_PREFIX_OUTPUT() \
  cout << OUTPUT_PREFIX << COLOR_CTL_ERROR << "error: " << COLOR_CTL_NORMAL
#define INTERNAL_SAY_PREFIX_OUTPUT() \
  cout << OUTPUT_PREFIX << COLOR_CTL_SAY << "out: " << COLOR_CTL_NORMAL

#define OUTPUT_PREFIX "twvm: "

// ANSI escape code (Colors);
#define COLOR_CTL_NORMAL "\x1b[37;40m"
#define COLOR_CTL_SAY "\x1b[92;40m"
#define COLOR_CTL_ERROR "\x1b[91;40m"
#define COLOR_CTL_DEBUG "\x1b[36;40m"
#define COLOR_CTL_WARNING "\x1b[93;40m"

using std::runtime_error;
using std::string;
using std::underlying_type;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::cout;
using std::endl;
using std::stringstream;
using std::istringstream;
using std::getline;
using std::memcpy;
using std::ostream;
using std::hex;
using std::showbase;

class Printer {
 private:
  vector<string> lines;

 public:
  inline void feedLine(const string &line) {
    lines.push_back(line);
  }
  inline void feedLine(stringstream &line) {
    lines.push_back(line.str());
    line.str(string());
  }
  void printTableView();
};

class Utils {
 public:
  inline static void debug(const string &msg = string(), bool hexFormat = false) {
    if (!CommandLine::isDebugMode) { return; }
    INTERNAL_DEBUG_PREFIX_OUTPUT();
    if (hexFormat) { cout << hex << showbase; }
    cout << msg << endl;
  }

  inline static void debug(const vector<string> msgs, bool hexFormat = false) {
    if (!CommandLine::isDebugMode) { return; }
    INTERNAL_DEBUG_PREFIX_OUTPUT();
    if (hexFormat) { cout << hex << showbase; }
    for (auto msg : msgs) { cout << msg; }
    cout << endl;
  }

  inline static ostream& say(const string &msg = string()) {
    INTERNAL_SAY_PREFIX_OUTPUT();
    return (msg.size() != 0) ? cout << msg << endl : cout;
  }

  inline static ostream& warn(const string &msg = string()) {
    INTERNAL_WARNING_PREFIX_OUTPUT();
    return (msg.size() != 0) ? cout << msg << endl : cout;
  }

  inline static ostream& report(
    const string &msg = "internal error occured.", bool throwException = true) {
    INTERNAL_ERROR_PREFIX_OUTPUT();
    cout << msg << endl;
    if (throwException) {
      throw runtime_error(msg);
    }
    return cout;
  }

  static vector<string> splitStr(const string&, char);

  static inline unique_ptr<Printer> getPrinter() {
    return make_unique<Printer>();
  }

  template <typename T>
  static inline void writeUnalignedValue(uintptr_t p, T value) {
    memcpy(reinterpret_cast<void*>(p), &value, sizeof(T));
  }

  template <typename T>
  static inline T readUnalignedValue(uintptr_t p) {
    T r;
    memcpy(&r, reinterpret_cast<void*>(p), sizeof(T));
    return r;
  }
};

#endif  // UTILS_H_
