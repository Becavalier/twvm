// Copyright 2020 YHSPY. All rights reserved.
#include <fstream>
#include <iostream>
#include <thread>
#include <utility>
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
    Loader::parseGlobalSection,
    Loader::parseExportSection,
    Loader::parseStartSection,
    Loader::parseElementSection,
    Loader::parseCodeSection,
    Loader::parseDataSection,
  };

  shared_module_t Loader::load(const std::string& fileName) {
    std::ifstream in {fileName, std::ifstream::binary};
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
      Exception::terminate(Exception::ErrorType::BAD_FSTREAM);
    }
    return wasmModule;
  }

  void Loader::parse(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    const auto sectionId = reader.currentSectionId();
    if (sectionId > 0) {
      handlers.at(sectionId)(reader, mod);
    }
  }

  void Loader::prelude(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    constexpr size_t totalHeaderBytes = MAGIC_BYTES_COUNT + VER_BYTES_COUNT;
    auto v = reader.retrieveBytes(totalHeaderBytes);
    const auto vSize = v.size();

    // incomplete magic bytes.
    if (vSize < MAGIC_BYTES_COUNT) 
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC);

    // validate magic code.
    auto parsedMagic = *reinterpret_cast<uint32_t*>(v.data());
    if (parsedMagic != VALID_MAGIC) {
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC, reader.pos());
    }

    // incomplete version bytes.
    if (vSize < totalHeaderBytes) 
      Exception::terminate(Exception::ErrorType::INVALID_VER); 

    // validate version code.
    auto parsedVersion = *reinterpret_cast<uint32_t*>(v.data() + MAGIC_BYTES_COUNT);
    if (parsedVersion != VALID_VERSION) {
      Exception::terminate(Exception::ErrorType::INVALID_VER, reader.pos());
    }
    mod->hasValidHeader = true;
    mod->version = parsedVersion;
  }

  void Loader::parseCustomSection(Reader& reader, shared_module_t mod) {
    const auto sectionSize = reader.walkU32();
    reader.skipBytes(sectionSize);
  }

  void Loader::parseTypeSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto entityCount = reader.walkU32();
    for (uint32_t i = 0; i < entityCount; ++i) {
      if (reader.walkU8() == Util::asInteger(LangTypes::FuncType)) {
        auto pair = std::make_pair(
          std::vector<int8_t>{}, 
          std::vector<int8_t>{});
        const auto paramsCount = reader.walkU32();
        for (uint32_t i = 0; i < paramsCount; ++i) {
          pair.first.push_back(reader.walkI8());
        }
        const auto resultCount = reader.walkU32();
        for (uint32_t i = 0; i < resultCount; ++i) {
          pair.second.push_back(reader.walkI8());
        }
        mod->funcTypes.emplace_back(std::move(pair));
      } else {
        Exception::terminate(Exception::ErrorType::INVALID_FUNC_TYPE, reader.pos());
      }
    }
  }

  void Loader::parseImportSection(Reader& reader, shared_module_t mod) {
    
  }

  void Loader::parseFunctionSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto funcIndexCount = reader.walkU32();
    for (uint32_t i = 0; i < funcIndexCount; ++i) {
      mod->funcTypesIndices.push_back(
        &mod->funcTypes.at(reader.walkU32())
      );
    }
  }

  void Loader::parseTableSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto tableCount = reader.walkU32();
    for (uint32_t i = 0; i < tableCount; ++i) {
      const auto elemType = reader.walkI8();
      const auto limitFlags = reader.walkU8();
      const auto limitInitial = reader.walkU32();
      mod->tables.emplace_back(elemType, limitInitial);
      if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
        mod->tables.back().maximum = reader.walkU32();
      }
    }
  }

  void Loader::parseMemorySection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto memCount = reader.walkU32();
    for (uint32_t i = 0; i < memCount; ++i) {
      const auto limitFlags = reader.walkU8();
      const auto limitInitial = reader.walkU32();
      mod->mems.emplace_back(limitInitial);
      if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
        mod->mems.back().maximum = reader.walkU32();
      }
    }
  }

  void Loader::parseStartSection(Reader& reader, shared_module_t mod) {
    mod->startFuncIdx = reader.walkU32();
  }

  void Loader::parseGlobalSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto globalCount = reader.walkU32();
    for (uint32_t i = 0; i < globalCount; ++i) {
      const auto valType = reader.walkI32();
    }
  }

  void Loader::parseExportSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto exportCount = reader.walkU32();
    for (uint32_t i = 0; i < exportCount; ++i) {
      const auto fieldLen = reader.walkU32();
      const auto fieldStr = reader.walkStringByBytes(fieldLen);
      const auto kind = reader.walkByte();
      const auto idx = reader.walkU32();
      mod->exports.emplace_back(fieldStr, kind, idx);
    }
  }

  void Loader::parseCodeSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto funcDefCount = reader.walkU32();
    for (uint32_t i = 0; i < funcDefCount; ++i) {
      const auto bodySize = reader.walkU32();
      const auto startPos = reader.pos();
      const auto locCount = reader.walkU32();
      std::vector<int8_t> locVarTypeVec = {};
      for (uint32_t j = 0; j < locCount; ++j) {
        const auto locVarCount = reader.walkU32();
        const auto locVarType = reader.walkI8();
        locVarTypeVec.insert(locVarTypeVec.end(), locVarCount, locVarType);
      }
      auto body = reader.retrieveBytes(bodySize - (reader.pos() - startPos));
      mod->funcDefs.emplace_back(std::move(locVarTypeVec), std::move(body));
    }
  }

  void Loader::parseElementSection(Reader& reader, shared_module_t mod) {
  }

  void Loader::parseDataSection(Reader& reader, shared_module_t mod) {
  }
}
