#ifndef TWVM_DECODER_H
#define TWVM_DECODER_H

#include <cstdint>
#include <cstring>
#include "types.h"

class Decoder {
 private:
  template <typename T>
  static inline T readLittleEndian(uchar_t* source) {
    // stack value;
    T r;
    // copying accordingly width;
    memcpy(&r, reinterpret_cast<void*>(source), sizeof(T));
    return r;
  }

 public:
  static uint8_t readU8(uchar_t* source);
  static uint16_t readU16(uchar_t* source);
  static uint32_t readU32(uchar_t* source);
  static uint64_t readU64(uchar_t* source);
};

#endif
