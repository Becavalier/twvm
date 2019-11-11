// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <stdexcept>
#include "./util.h"

using std::cerr;
using std::endl;
using std::runtime_error;

void Util::reportError(const string &msg, bool throwException) {
  cerr
    << OUTPUT_PREFIX
    << COLOR_CTL_ERROR
    << "error: "
    << COLOR_CTL_NORMAL
    << msg
    << endl;
  if (throwException) {
    throw runtime_error(msg);
  }
}

void Util::reportDebug(const string &msg) {
  cerr
    << OUTPUT_PREFIX
    << COLOR_CTL_DEBUG
    << "debug: "
    << COLOR_CTL_NORMAL
    << msg
    << endl;
}
