#include <memory>
#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "lib/loader.h"
#include "lib/executor.h"
#include "lib/instantiator.h"
#include "lib/inspector.h"

using std::make_unique;
using std::cout;
using std::endl;
using std::vector;

#define START_BYTES \
  0, 0x61, 0x73, 0x6d, 0x1, 0, 0, 0

TEST(TWVM, MemoryOperations) {
  const auto executor = make_unique<Executor>();
  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.store (i32.const 0) (i32.const 1))
        (i32.load (i32.const 0))
      ))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x10, 0x1, 0x0e, 0, 0x41, 0, 0x41, 0x1, 0x36, 0x2, 0, 0x41, 0,
        0x28, 0x2, 0, 0x0b,
      })));
  EXPECT_EQ(1, executor->inspectRunningResult<int32_t>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.store (i32.const 0) (i32.const -1))
        (i32.load (i32.const 0))
      ))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x10, 0x1, 0x0e, 0, 0x41, 0, 0x41, 0x7f, 0x36, 0x2, 0, 0x41, 0,
        0x28, 0x2, 0, 0x0b,
      })));
  EXPECT_EQ(-1, executor->inspectRunningResult<int32_t>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.store8 (i32.const 0) (i32.const 1000))
        (i32.load (i32.const 0))
      ))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x11, 0x1, 0xf, 0, 0x41, 0, 0x41, 0xe8, 0x7, 0x3a, 0, 0, 0x41,
        0, 0x28, 0x2, 0, 0xb,
      })));
  EXPECT_EQ(232, executor->inspectRunningResult<int32_t>());
}
