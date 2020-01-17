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

class WasmMemoryInstance {
 public:
  SET_STRUCT_MOVE_ONLY(WasmMemoryInstance);
  WasmMemoryInstance(uint32_t initMemSize = 1, uint32_t maxMemSize = 0) : maxMemSize(maxMemSize) {
    if (initMemSize > 0 && (maxMemSize == 0 || initMemSize <= maxMemSize)) {
      // allocate space (multiple of Wasm page);
      if ((data = static_cast<uint8_t*>(calloc(initMemSize * WASM_PAGE_SIZE, U8_SIZE)))) {
        currentMemSize = initMemSize;
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

  inline const auto availableSize() const {
    return currentMemSize * WASM_PAGE_SIZE;
  }

  inline const auto maxSize() const {
    return maxMemSize;
  }

  inline const auto& rawDataBuf() {
    return data;
  }

  // memory -> stack;
  template <typename T>
  T load(uint32_t offset) {
    if (offset + sizeof(T) <= availableSize()) {
      return *reinterpret_cast<T*>(data + offset);
    } else {
      Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
      // unreachable;
      return false;
    }
  }

  // stack -> memory;
  template <typename T>
  void store(uint32_t offset, T val) {
    // bound check;
    if (offset + sizeof(T) <= availableSize()) {
      *(reinterpret_cast<T*>(data + offset)) = val;
    } else {
      Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
    }
  }

  void inspect(uint32_t size = 20) const {
    for (uint32_t i = 0; i < size; i++) {
      cout << static_cast<int>(*(data + i)) << ' ';
    }
  }

  // expand the maxmium capacity of the memory by Wasm pages;
  const size_t grow(const size_t &pages) {
    if (maxMemSize != 0 && currentMemSize + pages > maxMemSize) {
      return -1;
    } else {
      const auto previousMemSize = currentMemSize;
      currentMemSize = currentMemSize + pages;
      // TODO(Jason Yu): relloc;
      return previousMemSize;
    }
  }

 private:
  // 64k per pages;
  uint32_t maxMemSize = 0;
  uint32_t currentMemSize = 0;
  uint8_t* data = nullptr;
  const WasmMemory *staticMemory;
};

#endif  // LIB_INSTANCES_INS_SECTION_MEMORY_H_
