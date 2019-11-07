#ifndef TWVM_LOADER_H
#define TWVM_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include "types.h"
#include "module.h"

using std::vector;
using std::string;
using std::shared_ptr;

class Loader {
 private:
  static vector<uchar_t> buf;
  static bool validateMagicWord(vector<uchar_t> &buf);
  static bool validateVersionWord(vector<uchar_t> &buf);
  static bool validateWords(vector<uchar_t> &buf);

  // analyzer helpers;

  
 public:
  static shared_ptr<Module> init(const string &file_name);
  static shared_ptr<Module> init(const uchar_t *source, size_t len);
};

#endif
