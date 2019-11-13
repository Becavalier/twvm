// Copyright 2019 YHSPY. All rights reserved.
#include <vector>
#include "src/decoder.h"
#include "src/constants.h"

using std::vector;

#define READ_AND_MOVE(module, T) \
  readLittleEndian<T>(module->getCurrentOffsetBuf(), \
    [&module](T val) -> void { \
      module->increaseBufOffset(sizeof(T)); \
    })

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

/**
 * LEB-128
 * MSB ------------------ LSB;
 * 00100110 10001110 11100101 624485 (Unsigned);
 * 01111000 10111011 11000000 -123456 (Signed);
 */
template <typename T>
T Decoder::readVarUint(const shared_module_t module) {
  if (sizeof(T) == 1) {
    return readUint8(module);
  } else {
    vector<uchar_t> t;
    while (true) {
      uchar_t d = READ_AND_MOVE(module, uchar_t);
      t.push_back(d);
      if (!(d & 0x80)) {
        break;
      }
    }
    T r = 0;
    unsigned shift = 0;
    for (auto byte : t) {
      r |= (static_cast<T>(byte & 0x7f) << shift);
      shift += 7;
    }
    return r;
  }
}

template <typename T>
T Decoder::readVarInt(const shared_module_t module) {
  vector<uchar_t> t;
  while (true) {
    uchar_t d = READ_AND_MOVE(module, uchar_t);
    t.push_back(d);
    if (!(d & 0x80)) {
      break;
    }
  }
  T r = 0;
  unsigned shift = 0;
  uchar_t b;
  for (auto byte : t) {
    b = byte;
    r |= (static_cast<T>(byte & 0x7f) << shift);
    shift += 7;
  }
  if ((shift < sizeof(T) * 8) && (b & 0x40)) {
    // sign extend;
    r |= (~0 << shift);
  }
  return r;
}

// varuint 1/7/32;
template uint8_t Decoder::readVarUint<uint8_t>(const shared_module_t);
template uint32_t Decoder::readVarUint<uint32_t>(const shared_module_t);
template uint64_t Decoder::readVarUint<uint64_t>(const shared_module_t);

// varint 7/32/64;
template int8_t Decoder::readVarInt<int8_t>(const shared_module_t);
template int32_t Decoder::readVarInt<int32_t>(const shared_module_t);
template int64_t Decoder::readVarInt<int64_t>(const shared_module_t);

string Decoder::decodeName(const shared_module_t module, size_t len) {
  auto name = readLittleEndian<char>(module->getCurrentOffsetBuf(), len,
    [&module, &len](char *val) -> void {
      module->increaseBufOffset(sizeof(char) * len);
    });
  return string(name, len);
}
