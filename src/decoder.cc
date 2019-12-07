// Copyright 2019 YHSPY. All rights reserved.
#include <vector>
#include "src/decoder.h"
#include "src/include/constants.h"

using std::vector;

uint8_t Decoder::readUint8(const uchar_t* source) {
  return readLittleEndian<uint8_t>(source);
}

uint8_t Decoder::readUint8(const shared_module_t module) {
  return READ_AND_MOVE(module, uint8_t);
}

uint16_t Decoder::readUint16(const uchar_t* source) {
  return readLittleEndian<uint16_t>(source);
}

uint16_t Decoder::readUint16(const shared_module_t module) {
  return READ_AND_MOVE(module, uint16_t);
}

uint32_t Decoder::readUint32(const uchar_t* source) {
  return readLittleEndian<uint32_t>(source);
}

uint32_t Decoder::readUint32(const shared_module_t module) {
  return READ_AND_MOVE(module, uint32_t);
}

uint64_t Decoder::readUint64(const uchar_t* source) {
  return readLittleEndian<uint64_t>(source);
}

uint64_t Decoder::readUint64(const shared_module_t module) {
  return READ_AND_MOVE(module, uint64_t);
}

string Decoder::decodeName(const shared_module_t module, size_t len) {
  auto name = readLittleEndian<char>(module->getCurrentOffsetBuf(), len,
    [&module, &len](char *val) -> void {
      module->increaseBufOffset(sizeof(char) * len);
    });
  return string(name, len);
}
