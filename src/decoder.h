// Copyright 2019 YHSPY. All rights reserved.
#ifndef DECODER_H_
#define DECODER_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <functional>
#include "./types.h"
#include "./module.h"

using std::function;
using std::string;
using std::memcpy;
using std::malloc;

class Decoder {
 private:
  template <typename T>
  static inline T readLittleEndian(
    const uchar_t* buf,
    const function<void(T)> &callback = nullptr) {
    T r;
    // copy amount of bytes accordingly;
    memcpy(&r, reinterpret_cast<const void*>(buf), sizeof(T));
    // callback function;
    if (callback) {
      callback(r);
    }
    return r;
  }

  template <typename T>
  static inline T* readLittleEndian(
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

 public:
  static uint8_t readUint8(const uchar_t*);
  static uint16_t readUint16(const uchar_t*);
  static uint32_t readUint32(const uchar_t*);
  static uint64_t readUint64(const uchar_t*);

  static uint8_t readUint8(const shared_module_t);
  static uint16_t readUint16(const shared_module_t);
  static uint32_t readUint32(const shared_module_t);
  static uint64_t readUint64(const shared_module_t);

  template <typename T>
  static T readVarUint(const shared_module_t);

  template <typename T>
  static T readVarInt(const shared_module_t);

  static string decodeName(const shared_module_t, size_t);
};

#endif  // DECODER_H_
