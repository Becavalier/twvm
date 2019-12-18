// Copyright 2019 YHSPY. All rights reserved.
#ifndef TWVM_H_
#define TWVM_H_

#include <chrono>
#include "src/include/errors.h"
#include "src/include/macros.h"
#include "src/include/constants.h"
#include "src/utility.h"
#include "src/loader.h"
#include "src/module.h"
#include "src/executor.h"
#include "src/stack.h"
#include "src/instantiator.h"
#include "src/inspector.h"
#include "src/cmdline.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

const auto calcTimeInterval(decltype(high_resolution_clock::now()) &previous) {
  const auto stop = high_resolution_clock::now();
  const auto duration = duration_cast<microseconds>(stop - previous);
  previous = stop;
  return duration.count() / 1000.0;
}

#endif  // TWVM_H_
