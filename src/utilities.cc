// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <stdexcept>
#include "./utilities.h"

using std::cerr;
using std::endl;
using std::runtime_error;

void Utilities::reportError(const string &msg, bool throwException) {
  REPORT(COLOR_CTL_ERROR, "error: ");
  if (throwException) {
    throw runtime_error(msg);
  }
}

void Utilities::reportDebug(const string &msg) {
  REPORT(COLOR_CTL_DEBUG, "debug: ");
}

void Utilities::reportWarning(const string &msg) {
  REPORT(COLOR_CTL_WARNING, "warning: ");
}
