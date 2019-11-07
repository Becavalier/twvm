// Copyright 2019 YHSPY. All rights reserved.
#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include "./types.h"

using std::vector;

class Module {
 public:
  Module() = default;
  ~Module() = default;

  void setModContent(const vector<uchar_t> &content) {
    buf = content;
  }

  size_t getModContentLength(void) {
    return buf.size();
  }

 private:
  vector<uchar_t> buf;
};

#endif  // MODULE_H_
