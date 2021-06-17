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
 * i32.wrap_i64
 * i32.trunc_f32_s
 * i32.trunc_f32_u
 * i32.trunc_f64_s
 * i32.trunc_f64_u
 * i64.extend_i32_s
 * i64.extend_i32_u
 * i64.trunc_f32_s
 * i64.trunc_f32_u
 * i64.trunc_f64_s
 * i64.trunc_f64_u
 * f32.convert_i32_s
 * f32.convert_i32_u
 * f32.convert_i64_s
 * f32.convert_i64_u
 * f32.demote_f64
 * f64.convert_i32_s
 * f64.convert_i32_u
 * f64.convert_i64_s
 * f64.convert_i64_u
 * f64.promote_f32
 * i32.reinterpret_f32
 * i64.reinterpret_f64
 * f32.reinterpret_i32
 * f64.reinterpret_i64
 */
TEST(TWVM, Convert) {
  const auto executor = make_unique<Executor>();
  
  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.add
          (i32.add
            (i32.trunc_f32_s (f32.const 123.456)) 
            (i32.trunc_f32_u (f32.const -12345.123))) 
           (i32.add
            (i32.trunc_f64_s (f64.const 123.456)) 
            (i32.trunc_f64_u (f64.const -12345.123))))))                                                   
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x27, 0x1, 0x25, 0, 0x43, 0x79, 0xe9, 0xf6, 0x42, 0xa8, 0x43, 0x7e, 0xe4,
        0x40, 0xc6, 0xa9, 0x6a, 0x44, 0x77, 0xbe, 0x9f, 0x1a, 0x2f, 0xdd, 0x5e, 0x40, 0xaa, 0x44, 0xb4,
        0xc8, 0x76, 0xbe, 0x8f, 0x1c, 0xc8, 0xc0, 0xab, 0x6a, 0x6a, 0x0b,
      })));
  EXPECT_EQ(-24444, executor->inspectRunningResult<int32_t>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i64)
        (i64.add
          (i64.add
            (i64.trunc_f32_s (f32.const 123.456)) 
            (i64.trunc_f32_u (f32.const -12345.123))) 
           (i64.add
            (i64.trunc_f64_s (f64.const 123.456)) 
            (i64.trunc_f64_u (f64.const -12345.123))))))                                                   
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7e, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0x27, 1, 0x25, 0, 0x43, 0x79, 0xe9, 0xf6, 0x42, 0xae, 0x43, 0x7e, 0xe4,
        0x40, 0xc6, 0xaf, 0x7c, 0x44, 0x77, 0xbe, 0x9f, 0x1a, 0x2f, 0xdd, 0x5e, 0x40, 0xb0, 0x44, 0xb4,
        0xc8, 0x76, 0xbe, 0x8f, 0x1c, 0xc8, 0xc0, 0xb1, 0x7c, 0x7c, 0x0b,
      })));
  EXPECT_EQ(-24444, executor->inspectRunningResult<int64_t>());

  /**
    (module
      (memory $0 1)
      (export "main" (func $main))
      (func $main (; 1 ;) (result i32)
        (i32.reinterpret_f32 (f32.const 123.123))))
   */
  executor->execute(
    Instantiator::instantiate(
      Loader::init(vector<uint8_t>{
        START_BYTES, 0x1, 0x5, 0x1, 0x60, 0, 0x1, 0x7f, 0x3,
        0x2, 0x1, 0, 0x5, 0x3, 0x1, 0, 0x1, 0x7, 0x8, 0x1, 0x4, 0x6d, 0x61, 0x69, 0x6e,
        0, 0, 0xa, 0xa, 0x1, 0x8, 0, 0x43, 0xfa, 0x3e, 0xf6, 0x42, 0xbc, 0x0b,
      })));
  EXPECT_EQ(1123434234, executor->inspectRunningResult<int32_t>());
}
