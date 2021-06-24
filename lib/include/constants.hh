// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_CONSTANTS_HH_
#define LIB_INCLUDE_CONSTANTS_HH_

#define IDX_ZERO 0
#define IDX_ONE 1
// ANSI escape code (Colors).
#define RESET_NORMAL "\e[0m"
#define COLOR_ERR "\e[0;31m"

constexpr uint8_t MAGIC_BYTES_COUNT = 4;
constexpr uint8_t VER_BYTES_COUNT = 4;
constexpr uint32_t VALID_MAGIC = 0x6D736100;
constexpr uint8_t VALID_VERSION = 0x1;
constexpr size_t WASM_PAGE_SIZE_IN_BYTE = 64 * 1024;
constexpr size_t WASM_MAX_PAGES = 1 << 16;
constexpr uint8_t EXT_KIND_FUNC = 0x0;
constexpr uint8_t EXT_KIND_TAB = 0x1;
constexpr uint8_t EXT_KIND_MEM = 0x2;
constexpr uint8_t EXT_KIND_GLB = 0x3;
constexpr uint32_t OPTIONAL_SYM_BOOL_TRUE = 1;
constexpr uint32_t OPTIONAL_SYM_BOOL_FALSE = 0;

/* Magic Num */
constexpr uint8_t MAGIC_CON_OPCODE_PLUS_TYPE = 0xc0;
constexpr uint8_t MAGIC_VAR_INDEX_PLUS_TYPE = 0x7f;

/* Others */
constexpr auto SCREEN_ARG_SETW_OFFSET = 23;
constexpr auto INPUT_ENTRY_KEY_NAME = "entryFuncName";
constexpr auto INPUT_ENTRY_KEY_ARG = "entryArgs";

#endif  // LIB_INCLUDE_CONSTANTS_HH_
