// Copyright 2019 YHSPY. All rights reserved.
#include <vector>
#include "src/decoder.h"
#include "src/include/constants.h"

using std::vector;

uint8_t Decoder::readUint8(const uint8_t* source, size_t *step) {
  if (step) {
    return readLittleEndian<uint8_t>(source,
      [&step](auto a) -> void { *step += 1; });
  } else {
    return readLittleEndian<uint8_t>(source);
  }
}

uint16_t Decoder::readUint16(const uint8_t* source, size_t *step) {
  if (step) {
    return readLittleEndian<uint16_t>(source,
      [&step](auto a) -> void { *step += 2; });
  } else {
    return readLittleEndian<uint16_t>(source);
  }
}

uint32_t Decoder::readUint32(const uint8_t* source, size_t *step) {
  if (step) {
    return readLittleEndian<uint32_t>(source,
      [&step](auto a) -> void { *step += 4; });
  } else {
    return readLittleEndian<uint32_t>(source);
  }
}

uint64_t Decoder::readUint64(const uint8_t* source, size_t *step) {
  if (step) {
    return readLittleEndian<uint64_t>(source,
      [&step](auto a) -> void { *step += 8; });
  } else {
    return readLittleEndian<uint64_t>(source);
  }
}

string Decoder::decodeName(const uint8_t* source, size_t len, size_t *step) {
  if (step) {
    return string(readLittleEndian<char>(source, len,
      [&step, &len](auto a) -> void { *step += sizeof(char) * len; }), len);
  } else {
    return string(readLittleEndian<char>(source, len), len);
  }
}
