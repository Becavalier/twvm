#include <iostream>
#include "loader.h"

#define INPUT_SRC_POS 1

int main(int argc, char **argv) {
  Loader::cli_load_file(argv[INPUT_SRC_POS]);

  return 0;
}
