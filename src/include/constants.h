// Copyright 2019 YHSPY. All rights reserved.
#ifndef INCLUDE_CONSTANTS_H_
#define INCLUDE_CONSTANTS_H_

#define DEFAULT_ELEMENT_INDEX 0
#define WASM_PAGE_SIZE 64 * 1024
#define WASM_VALUE_BIT_PATTERN_WIDTH 16
#define DEFAULT_BYTE_LENGTH 4
#define SCREEN_WIDTH 80

constexpr auto ptrSize = sizeof(uintptr_t);
constexpr auto charSize = sizeof(char);
constexpr auto i32Size = sizeof(int32_t);
constexpr auto i64Size = sizeof(int64_t);
constexpr auto f32Size = sizeof(float);
constexpr auto f64Size = sizeof(double);

#endif  // INCLUDE_CONSTANTS_H_
