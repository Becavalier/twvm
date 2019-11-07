#ifndef TWVM_MODULE_H
#define TWVM_MODULE_H

#include <vector>
#include "types.h"

using std::vector;

class Module {
 public:
  Module() = default;
  ~Module() = default;

  void setModContent(const vector<uchar_t> &content) {
    buf = content;
  };

  size_t getModContentLength(void) {
    return buf.size();
  }

 private:
  vector<uchar_t> buf;
};

#endif