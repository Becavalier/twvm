// Copyright 2019 YHSPY. All rights reserved.
#ifndef DECODER_H_
#define DECODER_H_

#define READ_AND_MOVE(module, T) \
  readLittleEndian<T>(module->getCurrentOffsetBuf(), \
    [&module](T val) -> void { \
      module->increaseBufOffset(sizeof(T)); \
    })

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include "src/utils.h"
#include "src/types.h"
#include "src/module.h"

using std::function;
using std::string;
using std::memcpy;
using std::malloc;
using std::vector;

class Decoder {
 private:
  template <typename T>
  static T readLittleEndian(
    const uchar_t* buf,
    const function<void(T)> &callback = nullptr) {
    auto r = Utils::readUnalignedValue<T>(reinterpret_cast<uintptr_t>(buf));
    // callback function;
    if (callback) {
      callback(r);
    }
    return r;
  }

  template <typename T>
  static T* readLittleEndian(
    const uchar_t* buf,
    const size_t len,
    const function<void(T*)> &callback = nullptr) {
    const size_t size = sizeof(T) * len;
    auto r = malloc(size);
    // copy amount of bytes accordingly;
    memcpy(r, reinterpret_cast<const void*>(buf), size);
    auto typedPointer = reinterpret_cast<T*>(r);
    // callback function;
    if (callback) {
      callback(typedPointer);
    }
    return typedPointer;
  }

  template <typename T>
  static T readVarUint_(
    vector<uchar_t> t,
    size_t *step = nullptr) {
    T r = 0;
    unsigned shift = 0;
    for (auto byte : t) {
      r |= (static_cast<T>(byte & 0x7f) << shift);
      shift += 7;
    }
    if (step) {
      *step += t.size();
    }
    return r;
  }

  template <typename T>
  static T readVarInt_(
    vector<uchar_t> t,
    size_t *step = nullptr) {
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
    if (step) {
      *step += t.size();
    }
    return r;
  }

  static vector<uchar_t> moduleWrapValue(const shared_module_t module) {
    vector<uchar_t> t;
    while (true) {
      uchar_t d = READ_AND_MOVE(module, uchar_t);
      t.push_back(d);
      if (!(d & 0x80)) {
        break;
      }
    }
    return t;
  }

  static vector<uchar_t> ptrWrapValue(const uchar_t *p) {
    vector<uchar_t> t;
    while (true) {
      uchar_t d = readLittleEndian<uchar_t>(p++);
      t.push_back(d);
      if (!(d & 0x80)) {
        break;
      }
    }
    return t;
  }

 public:
    /**
   * LEB-128
   * MSB ------------------ LSB;
   * 00100110 10001110 11100101 624485 (Unsigned);
   * 01111000 10111011 11000000 -123456 (Signed);
   */
  template <typename T>
  static T readVarUint(
    const shared_module_t module,
    size_t *step = nullptr) {
    if (sizeof(T) == 1) {
      return readUint8(module);
    } else {
      vector<uchar_t> t = Decoder::moduleWrapValue(module);
      return Decoder::readVarUint_<T>(t, step);
    }
  }

  template <typename T>
  static T readVarUint(
    const uchar_t *p,
    size_t *step = nullptr) {
    if (sizeof(T) == 1) {
      return readUint8(p);
    } else {
      vector<uchar_t> t = Decoder::ptrWrapValue(p);
      return Decoder::readVarUint_<T>(t, step);
    }
  }

  template <typename T>
  static T readVarInt(
    const shared_module_t module,
    size_t *step = nullptr) {
    vector<uchar_t> t = Decoder::moduleWrapValue(module);
    return Decoder::readVarInt_<T>(t, step);
  }

  template <typename T>
  static T readVarInt(
    const uchar_t *p,
    size_t *step = nullptr) {
    vector<uchar_t> t = Decoder::ptrWrapValue(p);
    return Decoder::readVarInt_<T>(t, step);
  }

  static size_t calcPassBytes(const uchar_t *p, size_t num = 1) {
    size_t total = 0;
    for (auto i = 0; i < num; i++) {
      total += ptrWrapValue(p).size();
    }
    return total;
  }

  static uint8_t readUint8(const uchar_t*);
  static uint16_t readUint16(const uchar_t*);
  static uint32_t readUint32(const uchar_t*);
  static uint64_t readUint64(const uchar_t*);

  static uint8_t readUint8(const shared_module_t);
  static uint16_t readUint16(const shared_module_t);
  static uint32_t readUint32(const shared_module_t);
  static uint64_t readUint64(const shared_module_t);

  static string decodeName(const shared_module_t, size_t);
};

#endif  // DECODER_H_
