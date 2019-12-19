#include <memory>
#include <iostream>
#include "gtest/gtest.h"
#include "lib/loader.h"
#include "lib/executor.h"
#include "lib/instantiator.h"
#include "lib/cmdline.h"

using std::make_unique;
using std::cout;
using std::endl;

TEST(TWVM, MemoryOperations) {
  CommandLine::isDebugMode = true;
  const auto executor = make_unique<Executor>();
  // little-endian;
  uint8_t moduleBinary[] = {0, 0x61, 0x73, 0x6d, 0x1, 0, 0, 0};
  executor->execute(
    Instantiator::instantiate(
      Loader::init(moduleBinary, sizeof(moduleBinary))));
}
