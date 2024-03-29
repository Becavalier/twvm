// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_READER_HH_
#define LIB_INCLUDE_READER_HH_

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include "lib/include/structs.hh"
#include "lib/include/decoder.hh"
#include "lib/include/constants.hh"
#include "lib/include/exception.hh"

#define WALK_FUNC_DEF(name, type, suffix) \
    type walk##name() { \
      return Decoder::decodeVar##suffix<type>(in); \
    }
#define DEFINE_WALK_FUNCS(V) \
  V(U8, uint8_t, uint) \
  V(U16, uint16_t, uint) \
  V(U32, uint32_t, uint) \
  V(I8, int8_t, int) \
  V(I16, int16_t, int) \
  V(I32, int32_t, int)

namespace TWVM {

class Reader {
  int8_t sectionId = 0;
  std::ifstream& in;
 public:
  auto currentSectionId() const { return sectionId; }
  Reader(std::ifstream& in, shared_module_t mod) : in(in) {
    if (mod->hasValidHeader) {
      // Check next section id.
      const auto parsedSectionId = static_cast<int8_t>(in.get());
      // To make sure the sections are organized in ASC sequence.
      if (parsedSectionId > 0 && parsedSectionId <= mod->lastParsedSectionId) {
        Exception::terminate(Exception::ErrorType::INVALID_SECTION_ID, in.tellg());
      }
      mod->lastParsedSectionId = parsedSectionId;
      sectionId = parsedSectionId;
    }
  }
  // Member functions.
  std::vector<uint8_t> retrieveBytes(size_t);
  std::vector<uint8_t> getBytesTillDelim(uint8_t);
  size_t pos() {
    return in.tellg();
  }
  uint8_t walkByte() {
    return static_cast<uint8_t>(in.get());
  }
  std::string walkStringByBytes(size_t n) {
    char str[n];
    in.read(str, n);
    return std::string(str, n);
  }
  void skipBytes(size_t n) {
    in.seekg(n, std::ios_base::cur);
  }
  DEFINE_WALK_FUNCS(WALK_FUNC_DEF)
};

}  // namespace TWVM

#endif  // LIB_INCLUDE_READER_HH_
