// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_INSTANCES_INS_SECTION_MEMORY_H_
#define LIB_INSTANCES_INS_SECTION_MEMORY_H_

#include <cstdlib>
#include "lib/utility.h"
#include "lib/type.h"
#include "lib/common/macros.h"
#include "lib/common/errors.h"

using std::calloc;
using std::free;
using std::realloc;

class WasmMemoryInstance {
 public:
  SET_STRUCT_MOVE_ONLY(WasmMemoryInstance);
  WasmMemoryInstance(uint32_t initMemSize = 1, uint32_t maxMemPage = 0) : maxMemPage(maxMemPage) {
    if (initMemSize > 0 && (maxMemPage == 0 || initMemSize <= maxMemPage)) {
      // allocate space (multiple of Wasm page).
      // TODO(Jason) use shared_ptr instead.
      if ((data = static_cast<uint8_t*>(calloc(initMemSize * WASM_PAGE_SIZE, U8_SIZE)))) {
        currentMemPage = initMemSize;
      } else {
        Printer::instance().error(Errors::LOADER_MEM_ALLOC_ERR);
      }
    } else {
      Printer::instance().error(Errors::LOADER_MEM_ALLOC_SIZE_ERR);
    }
  }
  ~WasmMemoryInstance() {
    free(data);
  }

  const auto getAvailableSize() const {
    return currentMemPage * WASM_PAGE_SIZE;
  }

  const auto getAvailablePage() const {
    return currentMemPage;
  }

  const auto& rawDataBuf() { return data; }

  // memory -> stack.
  template <typename T>
  T load(uint32_t offset) {
    if (offset + sizeof(T) <= getAvailableSize()) {
      return *reinterpret_cast<T*>(data + offset);
    } else {
      Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
      // unreachable.
      return false;
    }
  }

  // stack -> memory.
  template <typename T>
  void store(uint32_t offset, T val) {
    // bound check.
    if (offset + sizeof(T) <= getAvailableSize()) {
      *(reinterpret_cast<T*>(data + offset)) = val;
    } else {
      Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
    }
  }

  void inspect(uint32_t size = 20) const {
    for (uint32_t i = 0; i < size; ++i) {
      cout << static_cast<int>(*(data + i)) << ' ';
    }
  }

  // expand the maxmium capacity of the memory by Wasm pages.
  const uint32_t grow(const uint32_t pages) {
    if (maxMemPage != 0 && (currentMemPage + pages > maxMemPage)) {
      return -1;
    } else {
      const auto previousMemSize = currentMemPage;
      currentMemPage += pages;
      if (const auto &mem = realloc(data, currentMemPage * WASM_PAGE_SIZE)) {
        data = reinterpret_cast<uint8_t*>(mem);
        // initialization of the raw memeory.
      } else {
        Printer::instance().error(Errors::LOADER_MEM_ALLOC_ERR);
      }
      return previousMemSize;
    }
  }

 private:
  // 64k per pages.
  uint32_t maxMemPage = 0;
  uint32_t currentMemPage = 0;
  uint8_t* data = nullptr;
  const WasmMemory *staticMemory;
};

#endif  // LIB_INSTANCES_INS_SECTION_MEMORY_H_
