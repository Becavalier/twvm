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
#include <type_traits>

#define OUTPUT_PREFIX "twvm: "

// ANSI escape code (Colors);
#define COLOR_CTL_NORMAL "\x1b[37;40m"
#define COLOR_CTL_ERROR "\x1b[91;40m"
#define COLOR_CTL_DEBUG "\x1b[36;40m"
#define COLOR_CTL_WARNING "\x1b[93;40m"

using std::string;
using std::underlying_type;

class Utilities {
 public:
  static std::ostream& reportError(const string &msg = "internal error occured.", bool = true);
  static std::ostream& reportDebug(const string &msg = "");
  static std::ostream& reportWarning(const string &msg = "");
};

#endif  // UTILITIES_H_
