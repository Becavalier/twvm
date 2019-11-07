// Copyright 2019 YHSPY. All rights reserved.
#include "./decoder.h"

uint8_t Decoder::readU8(const uchar_t* source) {
  return readLittleEndian<uint8_t>(source);
}

uint16_t Decoder::readU16(const uchar_t* source) {
  return readLittleEndian<uint16_t>(source);
}

uint32_t Decoder::readU32(const uchar_t* source) {
  return readLittleEndian<uint32_t>(source);
}

uint64_t Decoder::readU64(const uchar_t* source) {
  return readLittleEndian<uint64_t>(source);
}

