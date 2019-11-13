// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <stdexcept>
#include "src/utilities.h"

using std::cout;
using std::endl;
using std::runtime_error;

// will throw exception if no "msg" provided;
std::ostream& Utilities::reportError(const string &msg, bool throwException) {
  REPORT(COLOR_CTL_ERROR, "error: ", msg);
  if (throwException) {
    throw runtime_error(msg);
  }
}

std::ostream& Utilities::reportDebug(const string &msg) {
  REPORT(COLOR_CTL_DEBUG, "info: ", msg);
}

std::ostream& Utilities::reportWarning(const string &msg) {
  REPORT(COLOR_CTL_WARNING, "warning: ", msg);
}
