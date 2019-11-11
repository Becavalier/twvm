// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <array>
#include <stdexcept>
#include "./loader.h"
#include "./constants.h"
#include "./decoder.h"
#include "./util.h"

#define WRAP_UINT_FIELD(keyName, type, module) \
  const auto keyName = Decoder::readVarUint<type>(module)
#define WRAP_UINT_FIELD_(type, module) \
  Decoder::readVarUint<type>(module)

#define WRAP_INT_FIELD(keyName, type, module) \
  const auto keyName = Decoder::readVarInt<type>(module)

vector<uchar_t> Loader::buf;

using std::ifstream;
using std::ios;
using std::make_shared;
using std::array;

// constant values;
CAST_ENUM_VAL(sectionTypesCode, kTypeSection);
CAST_ENUM_VAL(sectionTypesCode, kImportSection);
CAST_ENUM_VAL(sectionTypesCode, kFunctionSection);
CAST_ENUM_VAL(sectionTypesCode, kTableSection);
CAST_ENUM_VAL(sectionTypesCode, kMemorySection);
CAST_ENUM_VAL(sectionTypesCode, kGlobalSection);
CAST_ENUM_VAL(sectionTypesCode, kExportSection);
CAST_ENUM_VAL(sectionTypesCode, kStartSection);
CAST_ENUM_VAL(sectionTypesCode, kElementSection);
CAST_ENUM_VAL(sectionTypesCode, kCodeSection);
CAST_ENUM_VAL(sectionTypesCode, kDataSection);
CAST_ENUM_VAL(sectionTypesCode, kDataCountSection);
CAST_ENUM_VAL(sectionTypesCode, kExceptionSection);

CAST_ENUM_VAL(valueTypesCode, kFunc);

shared_module_t Loader::init(const std::string &fileName) {
  ifstream in(fileName, ios::binary);
  char d, counter = 1;
  shared_module_t wasmModule(new Module());

  if (in.is_open()) {
    while (in.good()) {
      in.read(&d, sizeof(d));
      buf.push_back(d);
      // checking magic word / version number;
      if (counter == BYTE_LENGTH_8) {
        if (!validateWords(buf)) {
          return wasmModule;
        }
      }
      counter++;
    }
  }

  in.close();
  if (!in.eof() && in.fail()) {
    Util::reportError("can not reading file.");
    return nullptr;
  }

  // wrapping and returning a module instance;
  wasmModule->setModContent(buf);
  // parsing;
  parse(wasmModule);

  return wasmModule;
}

shared_module_t Loader::init(const uchar_t *source, size_t len) {
  shared_module_t wasmModule;
  // one-time copying;
  buf = vector<uchar_t>(source, source + len);

  if (validateWords(buf)) {
    wasmModule->setModContent(buf);
  }
  return wasmModule;
}

void Loader::parse(const shared_module_t &module) {
  try {
    parseSection(module);
  } catch(const std::exception& e) {
    Util::reportError("exception occur, process terminated.");
  }
}

void Loader::parseSection(const shared_module_t &module) {
  while (!module->hasEnd()) {
    auto sectionCode = Decoder::readVarUint<uint8_t>(module);

    // TODO(Jason Yu): constant based switch;
    if (sectionCode == kTypeSectionCode) {
      parseTypeSection(module);
    } else if (sectionCode == kImportSectionCode) {
      parseImportSection(module);
    } else if (sectionCode == kFunctionSectionCode) {
      parseFunctionSection(module);
    } else if (sectionCode == kTableSectionCode) {
      parseTableSection(module);
    } else if (sectionCode == kMemorySectionCode) {
      parseMemorySection(module);
    } else if (sectionCode == kGlobalSectionCode) {
      parseGlobalSection(module);
    } else if (sectionCode == kExportSectionCode) {
      parseExportSection(module);
    } else if (sectionCode == kCodeSectionCode) {
      parseCodeSection(module);
    } else {
      // skip unknown sections;
      skipKnownSection(sectionCode, module);
    }
  }
}

void Loader::parseTypeSection(const shared_module_t &module) {
  Util::reportDebug("parsing type section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(entryCount, uint32_t, module);
  for (auto i = 0; i < entryCount; i++) {
    if (Decoder::readUint8(module) == kFuncCode) {
      vector<valueTypesCode> typesArr;
      WRAP_UINT_FIELD(paramsCount, uint32_t, module);
      for (auto j = 0; j < paramsCount; j++) {
        typesArr.push_back(static_cast<valueTypesCode>(Decoder::readUint8(module)));
      }
      WRAP_UINT_FIELD(returnCount, uint8_t, module);
      for (auto j = 0; j < returnCount; j++) {
        typesArr.push_back(static_cast<valueTypesCode>(Decoder::readUint8(module)));
      }
      module->addFuncSignature(new WasmFunctionSig(paramsCount, returnCount, typesArr.data()));
    } else {
      Util::reportError("type section code mismatch.", true);
    }
  }
}

void Loader::parseImportSection(const shared_module_t &module) {
  Util::reportDebug("parsing import section.");
}

void Loader::parseFunctionSection(const shared_module_t &module) {
  Util::reportDebug("parsing function section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(declaredFuncCount, uint32_t, module);
  for (auto i = 0; i < declaredFuncCount; i++) {
    // indices: uint32_t;
    WRAP_UINT_FIELD(sigIndex, uint32_t, module);
    auto sig = module->getFunctionSig(sigIndex);
    auto funcIndex = module->getFunction().size();
    module->addFunction({sig, funcIndex, sigIndex, nullptr, 0, false, false});
  }
}

void Loader::parseTableSection(const shared_module_t &module) {
  Util::reportDebug("parsing table section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(tableCount, uint32_t, module);
  for (auto i = 0; i < tableCount; i++) {
    // only support "anyfunc" in MVP (by default);
    WRAP_UINT_FIELD(tableType, uint8_t, module);
    WRAP_UINT_FIELD(tableFlags, uint8_t, module);

    // placement-new && move;
    module->getTable().emplace_back();
    auto *table = &module->getTable().back();

    // TODO(Jason Yu): support multiple table type;
    WRAP_UINT_FIELD(initialSize, uint32_t, module);
    table->initialSize = initialSize;

    // (0 : no /1: has) maximum field;
    if (tableFlags == kWasmTrue) {
      WRAP_UINT_FIELD(maximumSize, uint32_t, module);
      table->maximumSize = maximumSize;
      table->hasMaximumSize = true;
    }
  }
}

void Loader::parseMemorySection(const shared_module_t &module) {
  Util::reportDebug("parsing memory section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(memeoryCount, uint32_t, module);
  if (memeoryCount > 1) {
    Util::reportError("only support one memory in MVP.", true);
  } else {
    WRAP_UINT_FIELD(memoryFlags, uint8_t, module);
    auto memory = make_shared<WasmMemory>();

    WRAP_UINT_FIELD(initialPages, uint32_t, module);
    memory->initialPages = initialPages;
    // (0 : no /1: has) maximum field;
    if (memoryFlags == kWasmTrue) {
      WRAP_UINT_FIELD(maximumPages, uint32_t, module);
      memory->maximumPages = maximumPages;
      memory->hasMaximumPages = true;
    }
    module->addMemory(memory);
  }
}

void Loader::parseGlobalSection(const shared_module_t &module) {
  Util::reportDebug("parsing global section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(globalCount, uint32_t, module);
  for (auto i = 0; i < globalCount; i++) {
    const auto contentType = static_cast<valueTypesCode>(Decoder::readUint8(module));
    const auto mutability = Decoder::readUint8(module) == kWasmTrue;
    module->addGlobal({contentType, mutability, nullptr, false, false});
    // TODO(Jason Yu): analyze initialization expr;
    // ending with 0x0b;
  }
}

void Loader::parseExportSection(const shared_module_t &module) {
  Util::reportDebug("parsing export section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(exportCount, uint32_t, module);
  for (auto i = 0; i < exportCount; i++) {
    WRAP_UINT_FIELD(nameLen, uint32_t, module);
    const auto name = Decoder::decodeName(module, nameLen);
    const auto exportType = static_cast<externalTypesCode>(Decoder::readUint8(module));
    uint32_t index;
    switch (exportType) {
      case externalTypesCode::kExternalFunction: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        module->getFunction()[index].exported = true;
        break;
      }
      case externalTypesCode::kExternalTable: {
        // TODO(Jason Yu) ;
        break;
      }
      case externalTypesCode::kExternalMemory: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        const auto mem = module->getMemory();
        if (index != 0 || mem == nullptr) {
          Util::reportError("invalid memory index.", true);
        } else {
          mem->exported = true;
        }
        break;
      }
      case externalTypesCode::kExternalGlobal: {
        // TODO(Jason Yu) ;
        break;
      }
      default: {
        Util::reportError("wrong export type.", true);
      }
    }
    module->addExport({name, exportType, index});
  }
}

void Loader::parseCodeSection(const shared_module_t &module) {
  Util::reportDebug("parsing code section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(bodyCount, uint32_t, module);
  for (auto i = 0; i < bodyCount; i++) {
    WRAP_UINT_FIELD(bodySize, uint32_t, module);
    // update function body;
    auto func = module->getFunction()[module->getImportedFuncCount() + i];
    func.code = module->getCurrentOffsetBuf();
    func.codeLen = bodySize;
    module->increaseBufOffset(bodySize);
  }
}

void Loader::skipKnownSection(uint8_t sectionCode, const shared_module_t &module) {
  // WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  std::cout << (int)sectionCode << std::endl;
}

bool Loader::validateWords(const vector<uchar_t> &buf) {
  if (!validateMagicWord(buf)) {
    Util::reportError("invalid wasm magic word, expect 0x6d736100.");
    return false;
  }
  if (!validateVersionWord(buf)) {
    Util::reportError("invalid wasm version, expect 0x01.");
    return false;
  }
  return true;
}

bool Loader::validateMagicWord(const vector<unsigned char> &buf) {
  return Decoder::readUint32(buf.data()) == kWasmMagicWord;
}

bool Loader::validateVersionWord(const vector<unsigned char> &buf) {
  // set up offset;
  auto sp = buf.data() + BYTE_LENGTH_4;
  return Decoder::readUint32(sp) == kWasmVersion;
}
