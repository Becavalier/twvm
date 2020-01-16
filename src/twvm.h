// Copyright 2019 YHSPY. All rights reserved.
#ifndef TWVM_H_
#define TWVM_H_

#include <chrono>
#include "lib/utility.h"
#include "lib/loader.h"
#include "lib/executor.h"
#include "lib/instantiator.h"
#include "lib/common/errors.h"
#include "lib/common/macros.h"
#include "lib/common/constants.h"
#include "lib/structures/struct-module.h"
#include "lib/structures/struct-stack.h"

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
