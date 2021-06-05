// Copyright 2020 YHSPY. All rights reserved.
#include <fstream>
#include <iostream>
#include <thread>
#include "lib/constants.h"
#include "lib/loader.h"
#include "lib/decoder.h"
#include "lib/util.h"
#include "lib/exception.h"

namespace TWVM {
  std::vector<Loader::handlerType> Loader::handlers = {
    Loader::parseCustomSection,
    Loader::parseTypeSection,
    Loader::parseImportSection,
    Loader::parseFunctionSection,
    Loader::parseTableSection,
    Loader::parseMemorySection,
    Loader::parseStartSection,
    Loader::parseGlobalSection,
    Loader::parseExportSection,
    Loader::parseCodeSection,
    Loader::parseElementSection,
    Loader::parseDataSection,
  };

  shared_module_t Loader::init(const std::string& fileName) {
    std::ifstream in{fileName, std::ifstream::binary};
    auto wasmModule = std::make_shared<Module>();
    if (in.is_open() && in.good()) {
      prelude(in, wasmModule);
#if defined(OPT_PAR_LOADING)
      // TODO: loading with multi-threading.
#else
      // loading within the current thread.
      while (!in.eof()) {
        Loader::parse(in, wasmModule);
      }
#endif
    } else {
      std::cout << "bad";
    }
    return wasmModule;
  }

  void Loader::parse(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    const auto sectionId = reader.currentSectionId();
    handlers.at(sectionId)(reader, mod);
  }

  void Loader::prelude(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    constexpr size_t totalHeaderBytes = MAGIC_BYTES_COUNT + VER_BYTES_COUNT;
    auto v = reader.retrieveBytes(totalHeaderBytes);
    const auto vSize = v.size();

    // incomplete magic bytes.
    if (vSize < MAGIC_BYTES_COUNT) 
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC, vSize);

    // validate magic code.
    auto parsedMagic = *reinterpret_cast<uint32_t*>(v.data());
    if (parsedMagic != VALID_MAGIC) {
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC);
    }

    // incomplete version bytes.
    if (vSize < totalHeaderBytes) 
      Exception::terminate(Exception::ErrorType::INVALID_VER, vSize); 

    // validate version code.
    auto parsedVersion = *reinterpret_cast<uint32_t*>(v.data() + MAGIC_BYTES_COUNT);
    if (parsedVersion != VALID_VERSION) {
      Exception::terminate(Exception::ErrorType::INVALID_VER, MAGIC_BYTES_COUNT + 1);
    }
    mod->hasValidHeader = true;
    mod->version = parsedVersion;
  }

  void Loader::parseCustomSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseTypeSection(Reader& reader, shared_module_t mod) {
    reader.walkSectionSize();
    
  }

  void Loader::parseImportSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseFunctionSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseTableSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseMemorySection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseStartSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseGlobalSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseExportSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseCodeSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseElementSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseDataSection(Reader& reader, shared_module_t mod) {
  }
}
