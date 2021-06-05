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
  };
}

#endif  // LIB_DECODER_H_
