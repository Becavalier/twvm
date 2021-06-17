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
 * unreachable
 * nop
 * block
 * loop
 * if
 * else
 * end
 * br
 * br_if
 * br_table
 * return
 * call
 * call_indirect
 * drop
 * select
 */
TEST(TWVM, Control) {
  const auto executor = make_unique<Executor>();
  
  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result f32)
        (select (f32.const 1.5) (f32.const 2) (i32.const 1))))    
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7d, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x11, 0x1, 0xf, 0, 0x43, 0, 0, 0xc0, 0x3f, 0x43, 0, 0, 0,
        0x40, 0x41, 0x1, 0x1b, 0xb,
      })));
  EXPECT_EQ(2, executor->inspectRunningResult<float>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result f32)
        (select (f32.const 1.5) (f32.const 2) (i32.const 0))))    
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7d, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x11, 0x1, 0xf, 0, 0x43, 0, 0, 0xc0, 0x3f, 0x43, 0, 0, 0,
        0x40, 0x41, 0, 0x1b, 0xb,
      })));
  EXPECT_EQ(1.5, executor->inspectRunningResult<float>());

}



