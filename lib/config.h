// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_CONFIG_H_
#define LIB_CONFIG_H_

#include <string>

using std::string;

struct Config {
  static bool isDebugMode;
  static string executeModulePath;
};

#endif  // LIB_CONFIG_H_
