// Copyright 2019 YHSPY. All rights reserved.
#ifndef Utilities_H_
#define Utilities_H_

#define CAST_ENUM_VAL(className, enumKey) \
  const auto enumKey##Code = Utilities::castUnderlyingVal<className>(className::enumKey)

#define REPORT(color, label) \
  cerr \
    << OUTPUT_PREFIX \
    << color \
    << label \
    << COLOR_CTL_NORMAL \
    << msg \
    << endl

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
  static void reportError(const string&, bool = false);
  static void reportDebug(const string&);
  static void reportWarning(const string&);

  template <typename T, typename E>
  static auto castUnderlyingVal(const E &e) {
    return static_cast<typename underlying_type<E>::type>(e);
  }
};

#endif  // Utilities_H_
