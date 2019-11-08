// Copyright 2019 YHSPY. All rights reserved.
#include "./decoder.h"
#include "./constants.h"

#define INNER_LAMBDA(module, T) \
  [&module](T val) -> void { \
    module->increaseBufOffset(sizeof(T) / 8); \
  }

uint8_t Decoder::readU8(const uchar_t* source) {
  return readLittleEndian<uint8_t>(source);
}

uint8_t Decoder::readU8(const shared_module_t module) {
  return readLittleEndian<uint8_t>(
    module->getCurrentOffsetBuf(), INNER_LAMBDA(module, uint8_t));
}

uint16_t Decoder::readU16(const uchar_t* source) {
  return readLittleEndian<uint16_t>(source);
}

uint16_t Decoder::readU16(const shared_module_t module) {
  return readLittleEndian<uint16_t>(
    module->getCurrentOffsetBuf(), INNER_LAMBDA(module, uint16_t));
}

uint32_t Decoder::readU32(const uchar_t* source) {
  return readLittleEndian<uint32_t>(source);
}

uint32_t Decoder::readU32(const shared_module_t module) {
  return readLittleEndian<uint32_t>(
    module->getCurrentOffsetBuf(), INNER_LAMBDA(module, uint32_t));
}

uint64_t Decoder::readU64(const uchar_t* source) {
  return readLittleEndian<uint64_t>(source);
}

uint64_t Decoder::readU64(const shared_module_t module) {
  return readLittleEndian<uint64_t>(
    module->getCurrentOffsetBuf(), INNER_LAMBDA(module, uint64_t));
}
