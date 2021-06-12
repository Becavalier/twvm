// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_CONSTANTS_H_
#define LIB_CONSTANTS_H_

constexpr uint8_t MAGIC_BYTES_COUNT = 4;
constexpr uint8_t VER_BYTES_COUNT = 4;
constexpr uint32_t VALID_MAGIC = 0x6D736100;
constexpr uint8_t VALID_VERSION = 0x1;
constexpr size_t WASM_PAGE_SIZE = 64 * 1024;
constexpr uint8_t EXT_KIND_FUNC = 0x0;
constexpr uint8_t EXT_KIND_TAB = 0x1;
constexpr uint8_t EXT_KIND_MEM = 0x2;
constexpr uint8_t EXT_KIND_GLB = 0x3;

/* Magic Num */
constexpr uint8_t MAGIC_OPCODE_PLUS_TYPE = 0xc0;
constexpr uint8_t MAGIC_VAR_INDEX_PLUS_TYPE = 0x7f;

#endif  // LIB_CONSTANTS_H_
