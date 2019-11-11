// Copyright 2019 YHSPY. All rights reserved.
#ifndef UTIL_H_
#define UTIL_H_

#define CAST_ENUM_VAL(className, enumKey) \
  const auto enumKey##Code = Util::castUnderlyingVal<className>(className::enumKey)

#include <iostream>
#include <string>
#include <type_traits>

#define OUTPUT_PREFIX "twvm: "

// ANSI escape code (Colors);
#define COLOR_CTL_NORMAL "\x1b[37;40m"
#define COLOR_CTL_ERROR "\x1b[91;40m"
#define COLOR_CTL_DEBUG "\x1b[36;40m"

using std::string;
using std::underlying_type;

class Util {
 public:
  static void reportError(const string&, bool = false);
  static void reportDebug(const string&);

  template <typename T, typename E>
  static auto castUnderlyingVal(const E &e) {
    return static_cast<typename underlying_type<E>::type>(e);
  }
};

#endif  // UTIL_H_
