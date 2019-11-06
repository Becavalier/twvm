#ifndef TWVM_TYPES_H
#define TWVM_TYPES_H

#include <cstdint>

using uchar_t = unsigned char;

constexpr uint32_t k_wasm_magic = 0x6d736100;
constexpr uint32_t k_wasm_version = 0x01;

// basic types;
enum class value_types_code : uint8_t {
  k_void = 0x40,
  k_i32 = 0x7f,
  k_i64 = 0x7e,
  k_f32 = 0x7d,
  k_f64 = 0x7c,
  k_s128 = 0x7b,
  k_func_ref = 0x70, // block_type;
};

constexpr uint8_t k_func_type = 0x60;

// import/export;
enum class external_types_code : uint8_t {
  k_external_function = 0,
  k_external_table = 1,
  k_external_memory = 2,
  k_external_global = 3
};

// sections;
enum class section_types_code : int8_t {
  k_unknown_section = 0,
  k_type_section = 1,
  k_import_section = 2,
  k_function_section = 3,
  k_table_section = 4,
  k_memory_section = 5,
  k_global_section = 6,
  k_export_section = 7,
  k_start_section = 8,
  k_element_section = 9,
  k_code_section = 10,
  k_data_section = 11,
  k_data_count_section = 12,
  k_exception_section = 13
};

#endif
