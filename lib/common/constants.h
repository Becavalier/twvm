// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_COMMON_CONSTANTS_H_
#define LIB_COMMON_CONSTANTS_H_

constexpr u_char DEFAULT_ELEMENT_INDEX = 0;
constexpr size_t WASM_PAGE_SIZE = 64 * 1024;
constexpr u_char WASM_VALUE_BIT_PATTERN_WIDTH = 16;
constexpr u_char DEFAULT_BYTE_LENGTH = 4;
constexpr u_char SCREEN_WIDTH = 80;

// basic constants.
constexpr auto PTR_SIZE = sizeof(uintptr_t);
constexpr auto CHAR_SIZE = sizeof(char);
constexpr auto FLOAT_SIZE = sizeof(float);
constexpr auto DOUBLE_SIZE = sizeof(double);
constexpr auto U8_SIZE = sizeof(uint8_t);
constexpr auto I32_SIZE = sizeof(int32_t);
constexpr auto I64_SIZE = sizeof(int64_t);
constexpr auto F32_SIZE = sizeof(float);
constexpr auto F64_SIZE = sizeof(double);
constexpr size_t BYTE_LEN = 2 << 8;

#endif  // LIB_COMMON_CONSTANTS_H_
