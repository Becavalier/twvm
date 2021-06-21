#include <variant>
#include "gtest/gtest.h"
#include "lib/include/loader.hh"
#include "lib/include/instantiator.hh"
#include "lib/include/executor.hh"
#include "lib/include/structs.hh"

using namespace TWVM;

auto run(const std::string& path) {
  return Executor::execute(
    Instantiator::instantiate(
      Loader::load(path)));
}

#define COMMA ,
#define MOD_REL_PATH "../tests/modules/"
#define CONCAT_PREFIX(X) Runtime:: X
#define CONCAT_LIT_STR(X) MOD_REL_PATH #X 
#define DECLARE_RETURNABLE_TESTS(MOD_NAME, RET_TYPE, RET_VAL, CMP_OP) \
  TEST(TWVM, MOD_NAME) { \
    CMP_OP(std::get<CONCAT_PREFIX(RET_TYPE)>(*run(CONCAT_LIT_STR(MOD_NAME) ".wasm")), RET_VAL); \
  }

#define ITERATE_TESTCASES(V) \
  V(block, rt_i32_t, 10, EXPECT_EQ) \
  V(loop, rt_i32_t, 10, EXPECT_EQ) \
  V(if, rt_i32_t, 7, EXPECT_EQ) \
  V(else, rt_i32_t, 8, EXPECT_EQ) \
  V(br, rt_i32_t, 10, EXPECT_EQ) \
  V(br_if, rt_i32_t, 10, EXPECT_EQ) \
  V(br_table, rt_i32_t, 22, EXPECT_EQ) \
  V(return, rt_i32_t, 10, EXPECT_EQ) \
  V(call_indirect, rt_i32_t, 10, EXPECT_EQ) \
  V(drop, rt_i32_t, 10, EXPECT_EQ) \
  V(select, rt_i32_t, 20, EXPECT_EQ) \
  V(local, rt_f64_t, 10.123, EXPECT_DOUBLE_EQ) \
  V(global, rt_i32_t, 10, EXPECT_EQ) \
  V(i32_load_store, rt_i32_t, 10, EXPECT_EQ) \
  V(i64_load_store, rt_i64_t, 10, EXPECT_EQ) \
  V(f32_load_store, rt_f32_t, 12.34, EXPECT_FLOAT_EQ) \
  V(f64_load_store, rt_f64_t, 12.34, EXPECT_DOUBLE_EQ) \
  V(i32_load8_s, rt_i32_t, -16, EXPECT_EQ) \
  V(i32_load8_u, rt_i32_t, 240, EXPECT_EQ) \
  V(i32_load16_s, rt_i32_t, 27008, EXPECT_EQ) \
  V(i32_load16_u, rt_i32_t, 27008, EXPECT_EQ) \
  V(i64_load8_s, rt_i64_t, -128, EXPECT_EQ) \
  V(i64_load8_u, rt_i64_t, 128, EXPECT_EQ) \
  V(i64_load16_s, rt_i64_t, -10000, EXPECT_EQ) \
  V(i64_load16_u, rt_i64_t, 55536, EXPECT_EQ) \
  V(i64_load32_s, rt_i64_t, -10000, EXPECT_EQ) \
  V(i64_load32_u, rt_i64_t, 4294957296, EXPECT_EQ) \
  V(i32_store8, rt_i32_t, 240, EXPECT_EQ) \
  V(i32_store16, rt_i32_t, 55536, EXPECT_EQ) \
  V(i64_store8, rt_i64_t, 240, EXPECT_EQ) \
  V(i64_store16, rt_i64_t, 55536, EXPECT_EQ) \
  V(i64_store32, rt_i64_t, 4294957296, EXPECT_EQ) \
  V(memory_size, rt_i32_t, 10, EXPECT_EQ) \
  V(memory_grow, rt_i32_t, 101, EXPECT_EQ) \
  V(i32_eqz, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_eqz, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_eq, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_eq, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_ne, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_ne, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_lt_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_lt_u, rt_i32_t, 0, EXPECT_EQ) \
  V(i64_lt_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_lt_u, rt_i32_t, 0, EXPECT_EQ) \
  V(i32_gt_s, rt_i32_t, 0, EXPECT_EQ) \
  V(i32_gt_u, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_gt_s, rt_i32_t, 0, EXPECT_EQ) \
  V(i64_gt_u, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_le_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_le_u, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_le_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_le_u, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_ge_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_ge_u, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_ge_s, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_ge_u, rt_i32_t, 1, EXPECT_EQ) \
  V(f32_eq, rt_i32_t, 1, EXPECT_EQ) \
  V(f64_eq, rt_i32_t, 1, EXPECT_EQ) \
  V(f32_ne, rt_i32_t, 0, EXPECT_EQ) \
  V(f64_ne, rt_i32_t, 0, EXPECT_EQ) \
  V(f32_le, rt_i32_t, 1, EXPECT_EQ) \
  V(f64_le, rt_i32_t, 1, EXPECT_EQ) \
  V(i32_clz, rt_i32_t, 28, EXPECT_EQ) \
  V(i64_clz, rt_i64_t, 60, EXPECT_EQ) \
  V(i32_ctz, rt_i32_t, 1, EXPECT_EQ) \
  V(i64_ctz, rt_i64_t, 1, EXPECT_EQ) \
  V(i32_popcnt, rt_i32_t, 2, EXPECT_EQ) \
  V(i64_popcnt, rt_i64_t, 2, EXPECT_EQ) \
  V(i64_add, rt_i64_t, 20, EXPECT_EQ) \
  V(i64_sub, rt_i64_t, 0, EXPECT_EQ) \
  V(i64_mul, rt_i64_t, 100, EXPECT_EQ) \
  V(i64_div_s, rt_i64_t, -1, EXPECT_EQ) \
  V(i64_div_u, rt_i64_t, 1844674407370955160, EXPECT_EQ) \
  V(i64_rem_s, rt_i64_t, 0, EXPECT_EQ) \
  V(i64_rem_u, rt_i64_t, 6, EXPECT_EQ) \
  V(i64_and, rt_i64_t, 2, EXPECT_EQ) \
  V(i64_or, rt_i64_t, -2, EXPECT_EQ) \
  V(i64_xor, rt_i64_t, -4, EXPECT_EQ) \
  V(i64_shl, rt_i64_t, -10240, EXPECT_EQ) \
  V(i64_shr_s, rt_i64_t, -1, EXPECT_EQ) \
  V(i64_shr_u, rt_i64_t, 18014398509481983, EXPECT_EQ) \
  V(i64_rotl, rt_i64_t, -9217, EXPECT_EQ) \
  V(i64_rotr, rt_i64_t, -162129586585337857, EXPECT_EQ) \
  V(i32_add, rt_i32_t, 0, EXPECT_EQ) \
  V(i32_sub, rt_i32_t, -20, EXPECT_EQ) \
  V(i32_mul, rt_i32_t, -100, EXPECT_EQ) \
  V(i32_div_s, rt_i32_t, -1, EXPECT_EQ) \
  V(i32_div_u, rt_i32_t, 429496728, EXPECT_EQ) \
  V(i32_rem_s, rt_i32_t, 0, EXPECT_EQ) \
  V(i32_rem_u, rt_i32_t, 6, EXPECT_EQ) \
  V(i32_and, rt_i32_t, 2, EXPECT_EQ) \
  V(i32_or, rt_i32_t, -2, EXPECT_EQ) \
  V(i32_xor, rt_i32_t, -4, EXPECT_EQ) \
  V(i32_shl, rt_i32_t, -10240, EXPECT_EQ) \
  V(i32_shr_s, rt_i32_t, -1, EXPECT_EQ) \
  V(i32_shr_u, rt_i32_t, 4194303, EXPECT_EQ) \
  V(i32_rotl, rt_i32_t, -9217, EXPECT_EQ) \
  V(i32_rotr, rt_i32_t, -37748737, EXPECT_EQ) \
  V(f32_abs, rt_f32_t, 12.34f, EXPECT_FLOAT_EQ) \
  V(f32_neg, rt_f32_t, 12.34f, EXPECT_FLOAT_EQ) \
  V(f32_ceil, rt_f32_t, -12.f, EXPECT_FLOAT_EQ) \
  V(f32_floor, rt_f32_t, -13.f, EXPECT_FLOAT_EQ) \
  V(f32_trunc, rt_f32_t, -12.f, EXPECT_FLOAT_EQ) \
  V(f32_nearest, rt_f32_t, -12.f, EXPECT_FLOAT_EQ) \
  V(f32_sqrt, rt_f32_t, 3.512833595275879f, EXPECT_FLOAT_EQ) \
  V(f32_add, rt_f32_t, 24.68f, EXPECT_FLOAT_EQ) \
  V(f32_sub, rt_f32_t, 0.f, EXPECT_FLOAT_EQ) \
  V(f32_mul, rt_f32_t, 152.27560424804688f, EXPECT_FLOAT_EQ) \
  V(f32_div, rt_f32_t, 1.f, EXPECT_FLOAT_EQ) \
  V(f32_min, rt_f32_t, 12.33f, EXPECT_FLOAT_EQ) \
  V(f32_max, rt_f32_t, 12.34f, EXPECT_FLOAT_EQ) \
  V(f32_copysign, rt_f32_t, 12.1f, EXPECT_FLOAT_EQ) \
  V(f64_abs, rt_f64_t, 12.34, EXPECT_DOUBLE_EQ) \
  V(f64_ceil, rt_f64_t, -12., EXPECT_DOUBLE_EQ) \
  V(f64_floor, rt_f64_t, -13., EXPECT_DOUBLE_EQ) \
  V(f64_trunc, rt_f64_t, -12., EXPECT_DOUBLE_EQ) \
  V(f64_nearest, rt_f64_t, -12., EXPECT_DOUBLE_EQ) \
  V(f64_sqrt, rt_f64_t, 3.5128336140500593, EXPECT_DOUBLE_EQ) \
  V(f64_add, rt_f64_t, 24.68, EXPECT_DOUBLE_EQ) \
  V(f64_sub, rt_f64_t, 0., EXPECT_FLOAT_EQ) \
  V(f64_mul, rt_f64_t, 152.2756, EXPECT_DOUBLE_EQ) \
  V(f64_div, rt_f64_t, 1., EXPECT_DOUBLE_EQ) \
  V(f64_min, rt_f64_t, 12.33, EXPECT_DOUBLE_EQ) \
  V(f64_max, rt_f64_t, 12.34, EXPECT_DOUBLE_EQ) \
  V(f64_copysign, rt_f64_t, 12.33, EXPECT_DOUBLE_EQ) \
  V(i32_wrap_i64, rt_i32_t, -1097262461, EXPECT_EQ) \
  V(i32_trunc_f32_s, rt_i32_t, -12345678, EXPECT_EQ) \
  V(i32_trunc_f32_u, rt_i32_t, 12345678, EXPECT_EQ) \
  V(i32_trunc_f64_s, rt_i32_t, -12345678, EXPECT_EQ) \
  V(i32_trunc_f64_u, rt_i32_t, 12345678, EXPECT_EQ) \
  V(i64_extend_i32_s, rt_i64_t, -12345678, EXPECT_EQ) \
  V(i64_extend_i32_u, rt_i64_t, 4282621618, EXPECT_EQ) \
  V(i64_trunc_f32_s, rt_i64_t, -12345678, EXPECT_EQ) \
  V(i64_trunc_f32_u, rt_i64_t, 12345678, EXPECT_EQ) \
  V(i64_trunc_f64_s, rt_i64_t, -12345678, EXPECT_EQ) \
  V(i64_trunc_f64_u, rt_i64_t, 12345678, EXPECT_EQ) \
  V(f32_convert_i32_s, rt_f32_t, -12345678.f, EXPECT_EQ) \
  V(f32_convert_i32_u, rt_f32_t, 4282621696.f, EXPECT_EQ) \
  V(f32_convert_i64_s, rt_f32_t, -12345678.f, EXPECT_EQ) \
  V(f32_convert_i64_u, rt_f32_t, 18446744073709552000.f, EXPECT_EQ) \
  V(f32_demote_f64, rt_f32_t, -12345678407663616.f, EXPECT_EQ) \
  V(f64_convert_i32_s, rt_f64_t, -123456789., EXPECT_EQ) \
  V(f64_convert_i32_u, rt_f64_t, 4171510507., EXPECT_EQ) \
  V(f64_convert_i64_s, rt_f64_t, -123456789., EXPECT_EQ) \
  V(f64_convert_i64_u, rt_f64_t, 18446744073586094000., EXPECT_EQ) \
  V(f64_promote_f32, rt_f64_t, -12345679020032., EXPECT_EQ) \
  V(i32_reinterpret_f32, rt_i32_t, -885235378, EXPECT_EQ) \
  V(i64_reinterpret_f64, rt_i64_t, -4510482390845489152, EXPECT_EQ) \
  V(f32_reinterpret_i32, rt_f32_t, -0.0000034867075555666815f, EXPECT_FLOAT_EQ) \
  V(f64_reinterpret_i64, rt_f64_t, 2.1097938843731583e-300, EXPECT_DOUBLE_EQ) \

ITERATE_TESTCASES(DECLARE_RETURNABLE_TESTS)

TEST(TWVM, EXPECT_EXIT) {
  EXPECT_EXIT(run(CONCAT_LIT_STR(unreachable.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::UNREACHABLE));
  EXPECT_EXIT(run(CONCAT_LIT_STR(br_if_unreachable.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::UNREACHABLE));
  EXPECT_EXIT(run(CONCAT_LIT_STR(i32_trunc_f32_u_throw.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::FLOAT_UNREPRESENTABLE));
  EXPECT_EXIT(run(CONCAT_LIT_STR(i32_trunc_f64_u_throw.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::FLOAT_UNREPRESENTABLE));
  EXPECT_EXIT(run(CONCAT_LIT_STR(i64_trunc_f32_u_throw.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::FLOAT_UNREPRESENTABLE));
  EXPECT_EXIT(run(CONCAT_LIT_STR(i64_trunc_f64_u_throw.wasm)), testing::ExitedWithCode(1), Exception::getErrorMsg(Exception::ErrorType::FLOAT_UNREPRESENTABLE));
  EXPECT_NO_FATAL_FAILURE(run(CONCAT_LIT_STR(nop.wasm)));
}
