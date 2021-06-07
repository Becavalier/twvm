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
#include "lib/opcodes.h"

namespace TWVM {
  std::vector<Loader::handler_t> Loader::handlers = {
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

  void Loader::walkExtMeta(Reader& reader, Module::ext_meta_t& extMetaRef, uint8_t extKind) {
    switch (extKind) {
      case EXT_KIND_FUNC: {  // Function.
        extMetaRef = reader.walkU32();
      }
      case EXT_KIND_TAB: {  // Table.
        const auto elemType = reader.walkByte();
        const auto limitFlags = reader.walkByte();
        const auto limitInitial = reader.walkU32();
        Module::TableType tableType { elemType, limitInitial };
        if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
          tableType.maximum = reader.walkU32();
        }
        extMetaRef = tableType;
      }
      case EXT_KIND_MEM: {  // Memory.
        const auto limitFlags = reader.walkByte();
        const auto limitInitial = reader.walkU32();
        Module::MemType memType { limitInitial };
        if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
          memType.maximum = reader.walkU32();
        }
        extMetaRef = memType;
      }
      case EXT_KIND_GLB: {  // Global.
        const auto valType = reader.walkByte();
        const auto valMut = reader.walkByte() == 1;
        Module::GlobalType globalType { valType, valMut };
        extMetaRef = globalType;
      }
    }
  };

  shared_module_t Loader::load(const std::string& fileName) {
    std::ifstream in {fileName, std::ifstream::binary};
    auto wasmModule = std::make_shared<Module>();
    if (in.is_open() && in.good()) {
      preamble(in, wasmModule);
#if defined(OPT_PAR_LOADING)
      // TODO: loading with multi-threading.
#else
      // Loading within the current thread.
      while (!in.eof()) {
        Loader::parse(in, wasmModule);
      }
      in.close();
#endif
    } else {
      Exception::terminate(Exception::ErrorType::BAD_FSTREAM);
    }
    return wasmModule;
  }

  void Loader::parse(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    const auto sectionId = reader.currentSectionId();
    if (sectionId >= 0) {
      handlers.at(sectionId)(reader, mod);
    }
  }

  void Loader::preamble(std::ifstream& in, shared_module_t mod) {
    auto reader = Reader(in, mod);
    constexpr size_t totalHeaderBytes = MAGIC_BYTES_COUNT + VER_BYTES_COUNT;
    auto v = reader.retrieveBytes(totalHeaderBytes);
    const auto vSize = v.size();

    // Incomplete magic bytes.
    if (vSize < MAGIC_BYTES_COUNT) 
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC);

    // Validate magic code.
    auto parsedMagic = *reinterpret_cast<uint32_t*>(v.data());
    if (parsedMagic != VALID_MAGIC) {
      Exception::terminate(Exception::ErrorType::INVALID_MAGIC, reader.pos());
    }

    // Incomplete version bytes.
    if (vSize < totalHeaderBytes) 
      Exception::terminate(Exception::ErrorType::INVALID_VER); 

    // Validate version code.
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
      if (reader.walkByte() == Util::asInteger(LangTypes::FuncType)) {
        auto pair = std::make_pair(
          std::vector<uint8_t>{}, 
          std::vector<uint8_t>{});
        const auto paramsCount = reader.walkU32();
        for (uint32_t i = 0; i < paramsCount; ++i) {
          pair.first.push_back(reader.walkByte());
        }
        const auto resultCount = reader.walkU32();
        for (uint32_t i = 0; i < resultCount; ++i) {
          pair.second.push_back(reader.walkByte());
        }
        mod->funcTypes.emplace_back(std::move(pair));
      } else {
        Exception::terminate(Exception::ErrorType::INVALID_FUNC_TYPE, reader.pos());
      }
    }
  }

  void Loader::parseImportSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto importEntryCount = reader.walkU32();
    for (uint32_t i = 0; i < importEntryCount; ++i) {
      const auto modNameLen = reader.walkU32();
      const auto modNameStr = reader.walkStringByBytes(modNameLen);
      const auto fieldLen = reader.walkU32();
      const auto fieldStr = reader.walkStringByBytes(fieldLen);
      const auto extKind = reader.walkByte();  // The kind of definition being imported.
      mod->imports.emplace_back(modNameStr, fieldStr, extKind);
      // Process external meta.
      walkExtMeta(reader, mod->imports.back().extMeta, extKind);
    }
  }

  void Loader::parseExportSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto exportCount = reader.walkU32();
    for (uint32_t i = 0; i < exportCount; ++i) {
      const auto fieldLen = reader.walkU32();
      const auto fieldStr = reader.walkStringByBytes(fieldLen);
      const auto extKind = reader.walkByte();
      const auto extIdx = reader.walkU32();
      mod->exports.emplace_back(fieldStr, extKind, extIdx);
    }
  }

  void Loader::parseFunctionSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto funcIndexCount = reader.walkU32();
    for (uint32_t i = 0; i < funcIndexCount; ++i) {
      mod->funcTypesIndices.push_back(reader.walkU32());
    }
  }

  void Loader::parseTableSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto tableCount = reader.walkU32();
    for (uint32_t i = 0; i < tableCount; ++i) {
      const auto elemType = reader.walkByte();
      const auto limitFlags = reader.walkByte();
      const auto limitInitial = reader.walkU32();
      mod->tables.emplace_back(elemType, limitInitial);
      if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
        mod->tables.back().tableType.maximum = reader.walkU32();
      }
    }
  }

  void Loader::parseMemorySection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto memCount = reader.walkU32();
    for (uint32_t i = 0; i < memCount; ++i) {
      const auto limitFlags = reader.walkByte();
      const auto limitInitial = reader.walkU32();
      mod->mems.emplace_back(limitInitial);
      if (limitFlags == Util::asInteger(LimitFlags::MAX_EXIST)) {
        mod->mems.back().memType.maximum = reader.walkU32();
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
      const auto valType = reader.walkByte();
      const auto valMut = reader.walkByte() == 1;
      auto initExprOps = reader.getBytesTillDelim(Util::asInteger(OpCodes::End));
      mod->globals.emplace_back(valType, valMut, std::move(initExprOps));
    }
  }

  void Loader::parseCodeSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto funcDefCount = reader.walkU32();
    for (uint32_t i = 0; i < funcDefCount; ++i) {
      const auto bodySize = reader.walkU32();
      const auto startPos = reader.pos();
      const auto locCount = reader.walkU32();
      std::vector<uint8_t> locVarTypeVec = {};
      for (uint32_t j = 0; j < locCount; ++j) {
        const auto locVarCount = reader.walkU32();
        const auto locVarType = reader.walkByte();
        locVarTypeVec.insert(locVarTypeVec.end(), locVarCount, locVarType);
      }
      auto body = reader.retrieveBytes(bodySize - (reader.pos() - startPos));
      mod->funcDefs.emplace_back(std::move(locVarTypeVec), std::move(body));
    }
  }

  void Loader::parseElementSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto elemSegCount = reader.walkU32();
    for (uint32_t i = 0; i < elemSegCount; ++i) {
      const auto tblIdx = reader.walkU32();
      auto initExprOps = reader.getBytesTillDelim(Util::asInteger(OpCodes::End));
      const auto funcIndicesCount = reader.walkU32();
      std::vector<uint32_t> funcIndices = {};
      for (uint32_t j = 0; j < funcIndicesCount; ++j) {
        funcIndices.push_back(reader.walkU32());
      }
      mod->elements.emplace_back(tblIdx, std::move(initExprOps), std::move(funcIndices));
    }
  }

  void Loader::parseDataSection(Reader& reader, shared_module_t mod) {
    [[maybe_unused]] const auto sectionSize = reader.walkU32();
    const auto dataSegCount = reader.walkU32();
    for (uint32_t i = 0; i < dataSegCount; ++i) {
      const auto memIdx = reader.walkU32();
      auto initExprOps = reader.getBytesTillDelim(Util::asInteger(OpCodes::End));
      const auto size = reader.walkU32();
      auto data = reader.retrieveBytes(size);
      mod->data.emplace_back(memIdx, std::move(initExprOps), std::move(data));
    } 
  }
}
