#include "decoder.h"

uint8_t Decoder::read_u8(uchar_t* source) {
  return read_little_endian<uint8_t>(source);
}

uint16_t Decoder::read_u16(uchar_t* source) {
  return read_little_endian<uint16_t>(source);
}

uint32_t Decoder::read_u32(uchar_t* source) {
  return read_little_endian<uint32_t>(source);
}

uint64_t Decoder::read_u64(uchar_t* source) {
  return read_little_endian<uint64_t>(source);
}

