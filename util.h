#ifndef TWVM_UTIL_H
#define TWVM_UTIL_H

#include <iostream>
#include <string>

#define OUTPUT_PREFIX "twvm: "

// ANSI escape code (Colors);
#define COLOR_CTL_NORMAL "\x1b[37;40m"
#define COLOR_CTL_ERROR "\x1b[91;40m"
#define COLOR_CTL_DEBUG "\x1b[36;40m"

using std::string;

class Util {
 public:
  static void reportError(const string &msg);
  static void reportDebug(const string &msg);
};

#endif
