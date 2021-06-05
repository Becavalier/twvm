// Copyright 2021 YHSPY. All rights reserved.
#include "gtest/gtest.h"
#include "lib/config.h"

using ::testing::InitGoogleTest;

int main(int argc, char **argv) {
  Config::isDebugMode = true;
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
