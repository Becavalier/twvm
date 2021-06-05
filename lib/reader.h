// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_READER_H_
#define LIB_READER_H_

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include "lib/structs.h"
#include "lib/decoder.h"
#include "lib/constants.h"
#include "lib/exception.h"

namespace TWVM {
  class Reader {
    size_t sectionId = 0;
    std::ifstream& in;
   public: 
    auto currentSectionId() const { return sectionId; }
    Reader(std::ifstream& in, shared_module_t mod) : in(in) {
      if (mod->hasValidHeader) {
        // check next section id.
        const auto parsedSectionId = static_cast<int8_t>(in.get());
        if (parsedSectionId != 0 && parsedSectionId != mod->lastParsedSectionId + 1) {
          Exception::terminate(Exception::ErrorType::INVALID_SECTION_ID, in.tellg());
        }
        mod->lastParsedSectionId = parsedSectionId;
        sectionId = parsedSectionId;
      }
    }
    // member functions.
    std::vector<uint8_t> retrieveBytes(size_t);
    bool skipBytes(size_t n) {
      in.seekg(n, std::ios_base::cur);
      return in.rdstate() == std::ios_base::goodbit;
    }
    uint32_t walkSectionSize() {
      return Decoder::decodeVaruint<u_int32_t>(in);  // section size - u32.
    }
  };
}

#endif  // LIB_READER_H_
