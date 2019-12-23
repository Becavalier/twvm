// Copyright 2019 YHSPY. All rights reserved.
#include "gtest/gtest.h"
#include "lib/cmdline.h"

using ::testing::InitGoogleTest;

int main(int argc, char **argv) {
  CommandLine::isDebugMode = true;
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
