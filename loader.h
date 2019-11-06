#ifndef TWVM_LOADER_H
#define TWVM_LOADER_H

#define WASM_PAGE_SIZE 64 * 1024

#include <string>
#include <vector>
#include "types.h"

using std::vector;
using std::string;

class Loader {
 private:
  static vector<uchar_t> buf;
  static bool validate_magic(vector<uchar_t> &buf);

 public:
  static void cli_load_file(const string &file_name);
};

#endif
