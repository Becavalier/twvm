#include <memory>
#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "tests/macros.h"
#include "lib/include/loader.hh"
#include "lib/include/executor.hh"
#include "lib/include/instantiator.hh"
#include "lib/inspector.h"

using std::make_unique;
using std::cout;
using std::endl;
using std::vector;

#define START_BYTES \
  0, 0x61, 0x73, 0x6d, 0x1, 0, 0, 0

/**
 * Case ISAs:
 * 
 * i32.shl / i64.shl
 * i32.shr_s / i64.shr_s
 * i32.shr_u / i64.shr_u
 * i32.rotl / i64.rotl
 * i32.rotr / i64.rotr
 */
TEST(TWVM, Shift) {
  const auto executor = make_unique<Executor>();

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.shr_s (i32.const -40000)
        (i32.shr_u (i32.const -200) (i32.const 2)))))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0xf, 0x1, 0xd, 0, 0x41, 0xc0, 0xc7, 0x7d, 0x41, 0xb8, 0x7e, 0x41, 0x2,
        0x76, 0x75, 0x0b,
      })));
  EXPECT_EQ(-3, executor->inspectRunningResult<int32_t>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.rotl
          (i32.rotr 
            (i32.shl (i32.const 4000)(i32.const 100))
            (i32.const 600))
          (i32.const 21))))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x12, 0x1, 0x10, 0, 0x41, 0xa0, 0x1f, 0x41, 0xe4, 0, 0x74, 0x41, 0xd8,
        0x4, 0x78, 0x41, 0x15, 0x77, 0x0b,
      })));
  EXPECT_EQ(8000, executor->inspectRunningResult<int32_t>());
}



