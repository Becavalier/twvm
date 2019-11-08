// Copyright 2019 YHSPY. All rights reserved.
#ifndef MODULE_H_
#define MODULE_H_

#include <vector>
#include <iostream>
#include <memory>
#include "./types.h"
#include "./util.h"

using std::vector;
using std::shared_ptr;

class Module {
 public:
  Module() = default;
  ~Module() {
    Util::reportDebug("module has been destructed.");
  };

  void setModContent(const vector<uchar_t> &content) {
    buf = content.data();
    contentLength = content.size();
  }

  inline size_t getModContentLength(void) {
    return contentLength;
  }

  inline const uchar_t* getCurrentOffsetBuf(void) {
    return (buf + p);
  }

  inline void increaseBufOffset(size_t step) {
    p += step;
  }

  inline bool hasEnd() {
    return contentLength == p + 1;
  }

 private:
  const uchar_t *buf;
  size_t contentLength;
  // start from the first section;
  size_t p = 8;
};

using shared_module_t = shared_ptr<Module>;

#endif  // MODULE_H_
