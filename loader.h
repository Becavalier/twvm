// Copyright 2019 YHSPY. All rights reserved.
#ifndef LOADER_H_
#define LOADER_H_

#include <string>
#include <vector>
#include <memory>
#include "./types.h"
#include "./module.h"

using std::vector;
using std::string;
using std::shared_ptr;

class Loader {
 private:
  static vector<uchar_t> buf;
  static bool validateMagicWord(const vector<uchar_t>&);
  static bool validateVersionWord(const vector<uchar_t>&);
  static bool validateWords(const vector<uchar_t>&);

  // analyzer helpers;
  static void parse(const shared_module_t);
  static void parseSection(const shared_module_t);
 public:
  static shared_module_t init(const string&);
  static shared_module_t init(const uchar_t*, size_t);
};

#endif  // LOADER_H_
