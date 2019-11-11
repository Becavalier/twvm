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

  // analyzer invokers;
  static void parse(const shared_module_t);

  // analyzer helpers;
  static void parseSection(const shared_module_t);
  static void parseTypeSection(const shared_module_t);
  static void parseImportSection(const shared_module_t);
  static void parseFunctionSection(const shared_module_t);
  static void parseTableSection(const shared_module_t);
  static void parseMemorySection(const shared_module_t);
  static void parseExportSection(const shared_module_t);
  
 public:
  static shared_module_t init(const string&);
  static shared_module_t init(const uchar_t*, size_t);
};

#endif  // LOADER_H_
