// Copyright 2019 YHSPY. All rights reserved.
#ifndef UTILITIES_H_
#define UTILITIES_H_

#define REPORT(color, label, msg) \
  cout \
    << OUTPUT_PREFIX \
    << color \
    << label \
    << COLOR_CTL_NORMAL; \
  if (msg.size() > 0) cout << msg << endl; \
  return cout;

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <sstream>

#define OUTPUT_PREFIX "twvm: "

// ANSI escape code (Colors);
#define COLOR_CTL_NORMAL "\x1b[37;40m"
#define COLOR_CTL_ERROR "\x1b[91;40m"
#define COLOR_CTL_DEBUG "\x1b[36;40m"
#define COLOR_CTL_WARNING "\x1b[93;40m"

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

class Printer {
 private:
  vector<string> lines;

 public:
  void inline feedLine(const string &line) {
    lines.push_back(line);
  }
  void inline feedLine(stringstream &line) {
    lines.push_back(line.str());
    line.str(string());
  }
  void printTableView();
};

class Utilities {
 public:
  static std::ostream& reportDebug(const string &msg = "");
  static std::ostream& reportWarning(const string &msg = "");
  static std::ostream& reportError(const string &msg = "internal error occured.", bool = true);
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

#endif  // UTILITIES_H_
