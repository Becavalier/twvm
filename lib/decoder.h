// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_DECODER_H_
#define LIB_DECODER_H_

#include <fstream>

namespace TWVM {
  class Decoder {
    static std::vector<uint8_t> retrievePackedLEB128Bytes(std::ifstream&);
   public:
    template<typename T>
    static T decodeVaruint(std::ifstream& in) {
      const auto v = retrievePackedLEB128Bytes(in);
      T val = 0;
      unsigned shift = 0;
      for (auto byte : v) {
        val |= (static_cast<T>(byte & 0x7f) << shift);
        shift += 7;
      }
      return val;
    }
    template<typename T>
    static T decodeVarint(std::ifstream& in) {
      const auto v = retrievePackedLEB128Bytes(in);
      T val = 0;
      unsigned shift = 0;
      uint8_t b = 0;
      for (auto byte : v) {
        b = byte;
        val |= (static_cast<T>(byte & 0x7f) << shift);
        shift += 7;
      }
      if ((shift < sizeof(T) * 8) && (b & 0x40)) {
        // sign extend.
        val |= (~0 << shift);
      }
      return val;
    }
  };
}

#endif  // LIB_DECODER_H_
