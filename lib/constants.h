// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_CONSTANTS_H_
#define LIB_CONSTANTS_H_

constexpr uint8_t MAGIC_BYTES_COUNT = 4;
constexpr uint8_t VER_BYTES_COUNT = 4;
constexpr uint32_t VALID_MAGIC = 0x6D736100;
constexpr uint8_t VALID_VERSION = 0x1;
constexpr size_t WASM_PAGE_SIZE = 64 * 1024;

#endif  // LIB_CONSTANTS_H_
