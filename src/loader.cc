// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <array>
#include <stdexcept>
#include <iostream>
#include "src/loader.h"
#include "src/constants.h"
#include "src/utilities.h"

vector<uchar_t> Loader::buf;

using std::endl;
using std::hex;
using std::ifstream;
using std::ios;
using std::array;

shared_module_t Loader::init(const std::string &fileName) {
  Utilities::reportDebug("- [LOADING PHASE] -");
  ifstream in(fileName, ios::binary);
  char d;
  size_t counter = 1;
  shared_module_t wasmModule(new Module());

  if (in.is_open() && in.good()) {
    while (in.read(&d, sizeof(d))) {
      buf.push_back(d);

      // check magic word / version number in stream;
      if (counter == BYTE_LENGTH_8) {
        if (!validateWords(buf)) {
          return wasmModule;
        }
      }
      counter++;
    }
  } else {
    Utilities::reportError("can not reading file.", false);
    return nullptr;
  }
  in.close();

  // wrap and return a module instance;
  wasmModule->setModContent(buf);

  // parsing start;
  parse(wasmModule);
  return wasmModule;
}

shared_module_t Loader::init(const uchar_t *source, size_t len) {
  Utilities::reportDebug("- [LOADING PHASE] -");
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
    Utilities::reportError("exception occur, process terminated.", false);
  }
}

void Loader::parseSection(const shared_module_t &module) {
  while (!module->hasEnd()) {
    const auto sectionCode = Decoder::readVarUint<uint8_t>(module);
    const auto sectionCodeType = static_cast<SectionTypesCode>(sectionCode);

    switch (sectionCodeType) {
      case SectionTypesCode::kUnknownSection: { parseUnkownSection(sectionCode, module); break; }
      case SectionTypesCode::kTypeSection: { parseTypeSection(module); break; }
      case SectionTypesCode::kImportSection: { parseImportSection(module); break; }
      case SectionTypesCode::kFunctionSection: { parseFunctionSection(module); break; }
      case SectionTypesCode::kTableSection: { parseTableSection(module); break; }
      case SectionTypesCode::kMemorySection: { parseMemorySection(module); break; }
      case SectionTypesCode::kStartSection: { parseStartSection(module); break; }
      case SectionTypesCode::kGlobalSection: { parseGlobalSection(module); break; }
      case SectionTypesCode::kExportSection: { parseExportSection(module); break; }
      case SectionTypesCode::kCodeSection: { parseCodeSection(module); break; }
      case SectionTypesCode::kElementSection: { parseElementSection(module); break; }
      case SectionTypesCode::kDataSection: { parseDataSection(module); break; }
      default: {
        // skip unknown sections;
        skipKnownSection(sectionCode, module);
        break;
      }
    }
  }
}

void Loader::parseUnkownSection(uint8_t sectionCode, const shared_module_t &module) {
  // buzzle;
  Utilities::reportWarning()
    << "customized section code: 0x"
    << hex << static_cast<int>(sectionCode)
    << '.' << endl;
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  module->increaseBufOffset(payloadLen);
}

void Loader::parseTypeSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing type section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(entryCount, uint32_t, module);
  for (uint32_t i = 0; i < entryCount; i++) {
    if (static_cast<ValueTypesCode>(Decoder::readUint8(module)) == ValueTypesCode::kFunc) {
      // allocate vector on heap;
      module->getFunctionSig()->emplace_back();
      const auto sig = &module->getFunctionSig()->back();
      WRAP_UINT_FIELD(paramsCount, uint32_t, module);
      for (uint32_t j = 0; j < paramsCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(Decoder::readUint8(module)));
      }
      WRAP_UINT_FIELD(returnCount, uint8_t, module);
      for (uint32_t j = 0; j < returnCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(Decoder::readUint8(module)));
      }
      sig->index = i;
      sig->paramsCount = paramsCount;
      sig->returnCount = returnCount;
    } else {
      Utilities::reportError("type section code mismatch.");
    }
  }
}

void Loader::parseImportSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing import section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(importCount, uint32_t, module);
  for (auto i = 0; i < importCount; i++) {
    // set module name;
    WRAP_UINT_FIELD(moduleNameLen, uint32_t, module);
    const auto moduleName = Decoder::decodeName(module, moduleNameLen);

    // set field name;
    WRAP_UINT_FIELD(fieldNameLen, uint32_t, module);
    const auto fieldName = Decoder::decodeName(module, fieldNameLen);
    const auto importType = static_cast<ExternalTypesCode>(Decoder::readUint8(module));
    uint32_t index;
    switch (importType) {
      case ExternalTypesCode::kExternalFunction: {
        WRAP_UINT_FIELD(sigIndex, uint32_t, module);
        const auto sig = module->getFunctionSig(sigIndex);
        index = module->getFunction()->size();
        module->getImportedFuncCount()++;
        module->getFunction()->push_back({sig, index, sigIndex, nullptr, 0, true, false});
        break;
      }
      case ExternalTypesCode::kExternalTable: {
        // MVP: only support "anyfunc" (by default);
        const auto tableType = static_cast<ValueTypesCode>(Decoder::readUint8(module));
        if (tableType != ValueTypesCode::kFuncRef) {
          Utilities::reportError("only support \"anyfunc\" type in table.");
        }
        index = module->getTable()->size();
        module->getImportedTableCount()++;

        // insert new element by placement-new && move;
        module->getTable()->emplace_back();
        auto *table = &module->getTable()->back();
        table->imported = true;
        table->type = tableType;
        consumeTableParams(module, table);
        break;
      }
      case ExternalTypesCode::kExternalMemory: {
        consumeMemoryParams(module);
        break;
      }
      case ExternalTypesCode::kExternalGlobal: {
        // TODO(Jason Yu) ;
        break;
      }
      default: {
        Utilities::reportError("unkonwn import type.");
      }
    }
    module->getImport()->push_back({moduleName, fieldName, importType, index});
  }
}

void Loader::parseFunctionSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing function section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(declaredFuncCount, uint32_t, module);
  for (auto i = 0; i < declaredFuncCount; i++) {
    module->getFunction()->emplace_back();
    const auto func = &module->getFunction()->back();
    // indices: uint32_t;
    WRAP_UINT_FIELD(sigIndex, uint32_t, module);
    const auto sig = module->getFunctionSig(sigIndex);
    const auto funcIndex = module->getFunction()->size();
    func->sig = sig;
    func->sigIndex = sigIndex;
    func->funcIndex = funcIndex;
  }
}

void Loader::parseTableSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing table section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(tableCount, uint32_t, module);
  for (auto i = 0; i < tableCount; i++) {
    // MVP: only support "anyfunc" (by default);
    const auto tableType = static_cast<ValueTypesCode>(Decoder::readUint8(module));
    if (tableType != ValueTypesCode::kFuncRef) {
      Utilities::reportError("only support \"anyfunc\" type in table.");
    }

    // insert new element by placement-new && move;
    module->getTable()->emplace_back();
    auto *table = &module->getTable()->back();
    table->type = tableType;
    consumeTableParams(module, table);
  }
}

void Loader::parseMemorySection(const shared_module_t &module) {
  Utilities::reportDebug("parsing memory section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(memeoryCount, uint32_t, module);

  // determine whether the memory has been initialized via "import";
  if (memeoryCount > 1 || module->getMemory() != nullptr) {
    Utilities::reportError("only support one memory in MVP.");
  } else {
    consumeMemoryParams(module);
  }
}

void Loader::parseStartSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing start section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(startFuncIndex, uint32_t, module);

  // start function: no arguments or return value;
  const auto sig = module->getFunction(startFuncIndex)->sig;
  if (sig->paramsCount != 0 || sig->returnCount != 0) {
    Utilities::reportError("the start function must not take any arguments or return value.");
  } else {
    module->getStartFuncIndex() = startFuncIndex;
  }
}

void Loader::parseGlobalSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing global section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(globalCount, uint32_t, module);
  for (auto i = 0; i < globalCount; i++) {
    const auto contentType = static_cast<ValueTypesCode>(Decoder::readUint8(module));
    const auto mutability = Decoder::readUint8(module) == kWasmTrue;

    // insert new element by placement-new && move;
    module->getGlobal()->emplace_back();
    auto *thisGlobal = &module->getGlobal()->back();
    thisGlobal->type = contentType;
    thisGlobal->mutability = mutability;

    // deal with opcode;
    consumeInitExpr(module, &thisGlobal->init);
  }
}

void Loader::parseExportSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing export section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(exportCount, uint32_t, module);
  for (auto i = 0; i < exportCount; i++) {
    WRAP_UINT_FIELD(nameLen, uint32_t, module);
    const auto name = Decoder::decodeName(module, nameLen);
    const auto exportType = static_cast<ExternalTypesCode>(Decoder::readUint8(module));
    uint32_t index;
    switch (exportType) {
      case ExternalTypesCode::kExternalFunction: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        const auto funcIns = module->getFunction(index);
        funcIns->exported = true;
        break;
      }
      case ExternalTypesCode::kExternalTable: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        const auto table = module->getTable(index);
        if (table != nullptr) {
          table->exported = true;
        }
        break;
      }
      case ExternalTypesCode::kExternalMemory: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        const auto mem = module->getMemory();
        if (index != 0 || mem == nullptr) {
          Utilities::reportError("invalid memory index.");
        } else {
          mem->exported = true;
        }
        break;
      }
      case ExternalTypesCode::kExternalGlobal: {
        index = WRAP_UINT_FIELD_(uint32_t, module);
        const auto global = module->getGlobal(index);
        if (global != nullptr) {
          global->exported = true;
        }
        break;
      }
      default: {
        Utilities::reportError("unkonwn export type.");
      }
    }
    module->getExport()->push_back({name, exportType, index});
  }
}

void Loader::parseCodeSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing code section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(bodyCount, uint32_t, module);
  for (auto i = 0; i < bodyCount; i++) {
    WRAP_UINT_FIELD(bodySize, uint32_t, module);
    // update function body;
    const auto function = module->getFunction(module->getImportedFuncCount() + i);
    function->code = module->getCurrentOffsetBuf();
    function->codeLen = bodySize;
    module->increaseBufOffset(bodySize);
  }
}

// initialize imported/internally-defined table;
void Loader::parseElementSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing element section.");
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(elemCount, uint32_t, module);
  if (elemCount > 0 && module->getTable()->size() == 0) {
    Utilities::reportError("no table found to apply the element section.");
  }
  for (auto i = 0; i < elemCount; i++) {
    WRAP_UINT_FIELD(tableIndex, uint32_t, module);
    if (tableIndex != 0) {
      Utilities::reportError("can only manipulate the default table in MVP.");
    }
    module->getElement()->emplace_back();
    auto *elem = &module->getElement()->back();
    elem->tableIndex = tableIndex;

    // deal with opcode;
    consumeInitExpr(module, &elem->init);

    // # of the element exprs;
    WRAP_UINT_FIELD(elemExprCount, uint32_t, module);
    for (auto j = 0; j < elemExprCount; j++) {
      WRAP_UINT_FIELD(index, uint32_t, module);
      elem->entities.push_back(module->getFunction(index));
    }
  }
}

void Loader::parseDataSection(const shared_module_t &module) {
  Utilities::reportDebug("parsing data section.");
}

void Loader::skipKnownSection(uint8_t byte, const shared_module_t &module) {
  Utilities::reportError()
    << "unknown byte found: 0x"
    << hex << static_cast<int>(byte)
    << '.' << endl;
  WRAP_UINT_FIELD(payloadLen, uint32_t, module);
}

bool Loader::validateWords(const vector<uchar_t> &buf) {
  if (!validateMagicWord(buf)) {
    Utilities::reportError("invalid wasm magic word, expect 0x6d736100.", false);
    return false;
  }
  if (!validateVersionWord(buf)) {
    Utilities::reportError("invalid wasm version, expect 0x01.", false);
    return false;
  }
  return true;
}

bool Loader::validateMagicWord(const vector<unsigned char> &buf) {
  return Decoder::readUint32(buf.data()) == kWasmMagicWord;
}

bool Loader::validateVersionWord(const vector<unsigned char> &buf) {
  // set offset;
  auto sp = buf.data() + BYTE_LENGTH_4;
  return Decoder::readUint32(sp) == kWasmVersion;
}
