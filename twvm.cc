#include <iostream>
#include "util.h"
#include "constants.h"
#include "loader.h"
#include "module.h"
#include "executor.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    Util::reportError("no input file.");
    return 1;
  }

  const auto wasmModule = Loader::init(argv[INPUT_ARG_OFFSET]);

  if (wasmModule->getModContentLength() > 0) {
    Util::reportDebug("file loaded.");
  }

  return 0;
}
