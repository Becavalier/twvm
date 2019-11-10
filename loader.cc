// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <array>
#include "./loader.h"
#include "./constants.h"
#include "./decoder.h"
#include "./util.h"

#define WRAP_UINT_FIELD(keyName, type, module) \
  const auto keyName = Decoder::readVarUint<type>(module)

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

void Loader::parse(const shared_module_t module) {
  parseSection(module);
}

void Loader::parseSection(const shared_module_t module) {
  // while (!module->hasEnd()) {
    auto sectionCode = Decoder::readVarUint<uint8_t>(module);

    if (sectionCode == kTypeSectionCode) {
      parseTypeSection(module);
    }
    if (sectionCode == kImportSectionCode) {
      parseImportSection(module);
    }
    if (sectionCode == kFunctionSectionCode) {
      parseFunctionSection(module);
    }
  // }
}

void Loader::parseTypeSection(const shared_module_t module) {
  Util::reportDebug("parsing type section.");
  /**
   * count varuint32;
   * entries [func_type];    
   */
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
      module->addFuncSignatures(new WasmFunctionSig(paramsCount, returnCount, typesArr.data()));
    } else {
      Util::reportError("type section code mismatch.");
    }
  }
}

void Loader::parseImportSection(const shared_module_t module) {
}

void Loader::parseFunctionSection(const shared_module_t module) {
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
