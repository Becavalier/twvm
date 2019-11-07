// Copyright 2019 YHSPY. All rights reserved.
#ifndef DECODER_H_
#define DECODER_H_

#include <cstdint>
#include <cstring>
#include "./types.h"

class Decoder {
 private:
  template <typename T>
  static inline T readLittleEndian(const uchar_t* source) {
    T r;
    // copying width accordingly;
    memcpy(&r, reinterpret_cast<const void*>(source), sizeof(T));
    return r;
  }

 public:
  static uint8_t readU8(const uchar_t* source);
  static uint16_t readU16(const uchar_t* source);
  static uint32_t readU32(const uchar_t* source);
  static uint64_t readU64(const uchar_t* source);
};

#endif  // DECODER_H_
