#ifndef TWVM_DECODER_H
#define TWVM_DECODER_H

#include <cstdint>
#include <cstring>
#include "types.h"

class Decoder {
 private:
  template <typename num_type>
  static inline num_type read_little_endian(uchar_t* source) {
    // stack value;
    num_type r;
    // copying accordingly width;
    memcpy(&r, reinterpret_cast<void*>(source), sizeof(num_type));
    return r;
  }

 public:
  static uint8_t read_u8(uchar_t* source);
  static uint16_t read_u16(uchar_t* source);
  static uint32_t read_u32(uchar_t* source);
  static uint64_t read_u64(uchar_t* source);
};

#endif
