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
  static bool validateMagicWord(const vector<uchar_t> &buf);
  static bool validateVersionWord(const vector<uchar_t> &buf);
  static bool validateWords(const vector<uchar_t> &buf);

  // analyzer helpers;


 public:
  static shared_ptr<Module> init(const string &fileName);
  static shared_ptr<Module> init(const uchar_t *source, size_t len);
};

#endif  // LOADER_H_
