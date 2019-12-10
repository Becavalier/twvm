// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <array>
#include <stdexcept>
#include <iostream>
#include "src/loader.h"
#include "src/include/constants.h"
#include "src/utils.h"

vector<uchar_t> Loader::buf;
ifstream* Loader::reader;
uint32_t Loader::byteCounter = 0;

using std::endl;
using std::hex;
using std::ifstream;
using std::ios;
using std::array;

shared_module_t Loader::init(const std::string &fileName) {
  (Printer::instance() << "- [LOADING PHASE] -\n").debug();
  ifstream in(fileName, ios::binary);
  shared_module_t wasmModule = make_shared<Module>();
  if (in.is_open() && in.good()) {
    reader = &in;
    // magic# and verison validations;
    retrieveBytes(BYTE_LENGTH_8);
    if (Decoder::readUint32(buf.data()) != kWasmMagicWord) {
      (Printer::instance() << "invalid wasm magic word, expect 0x6d736100.\n").error();
    }
    // offset;
    if (Decoder::readUint32(buf.data() + BYTE_LENGTH_4) != kWasmVersion) {
      (Printer::instance() << "invalid wasm version, expect 0x01.\n").error();
    }
  } else {
    (Printer::instance() << "can not reading file.\n").error();
  }

  // wrap and return a module instance;
  // wasmModule->setModContent(buf);

  // parsing start;
  parse(wasmModule);
  return wasmModule;
}

void Loader::parse(const shared_module_t &module) {
  parseSection(module);
}

void Loader::parseSection(const shared_module_t &module) {
  while (!module->hasEnd()) {
    const auto sectionCode = Decoder::readVarUint<uint8_t>(reader);
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
  Printer::instance().useHexFormat()
    << "customized section code: " << static_cast<int>(sectionCode) << ".\n";
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  module->increaseBufOffset(payloadLen);
}

void Loader::parseTypeSection(const shared_module_t &module) {
  (Printer::instance() << "parsing type section.\n").debug();
  // payload length;
  retrieveBytes(Decoder::readVarUint<uint32_t>(reader));
  const auto &sectionDataBuf = buf.data();
  const auto entryCount = Decoder::readVarUint<uint32_t>(sectionDataBuf);
  for (uint32_t i = 0; i < entryCount; i++) {
    if (static_cast<ValueTypesCode>(Decoder::readUint8(sectionDataBuf)) == ValueTypesCode::kFunc) {
      // allocate vector on heap;
      module->getFunctionSig()->emplace_back();
      const auto sig = &module->getFunctionSig()->back();
      WRAP_UINT_FIELD(paramsCount, uint32_t, sectionDataBuf);
      for (uint32_t j = 0; j < paramsCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(Decoder::readUint8(module)));
      }
      WRAP_UINT_FIELD(returnCount, uint8_t, sectionDataBuf);
      for (uint32_t j = 0; j < returnCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(Decoder::readUint8(module)));
      }
      sig->index = i;
      sig->paramsCount = paramsCount;
      sig->returnCount = returnCount;
    } else {
      (Printer::instance() << "type section code mismatch.\n").error();
    }
  }
}

void Loader::parseImportSection(const shared_module_t &module) {
  (Printer::instance() << "parsing import section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(importCount, uint32_t, module);
  for (uint32_t i = 0; i < importCount; i++) {
    // set module name;
    WRAP_UINT_FIELD(moduleNameLen, uint32_t, module);
    const auto moduleName = Decoder::decodeName(module, moduleNameLen);

    // set field name;
    WRAP_UINT_FIELD(fieldNameLen, uint32_t, module);
    const auto fieldName = Decoder::decodeName(module, fieldNameLen);
    const auto importType = static_cast<ExternalTypesCode>(Decoder::readUint8(module));
    uint32_t index = 0;
    switch (importType) {
      case ExternalTypesCode::kExternalFunction: {
        WRAP_UINT_FIELD(sigIndex, uint32_t, module);
        const auto sig = module->getFunctionSig(sigIndex);
        index = module->getFunction()->size();
        module->importedFuncCount++;
        module->getFunction()->push_back({sig, index, sigIndex, {}, nullptr, 0, true, false});
        break;
      }
      case ExternalTypesCode::kExternalTable: {
        // MVP: only support "anyfunc" (by default);
        const auto tableType = static_cast<ValueTypesCode>(Decoder::readUint8(module));
        if (tableType != ValueTypesCode::kFuncRef) {
          (Printer::instance() << "only support \"anyfunc\" type in table.\n").error();
        }
        index = module->getTable()->size();
        module->importedTableCount++;

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
        (Printer::instance() << "unkonwn import type.\n").error();
      }
    }
    module->getImport()->push_back({moduleName, fieldName, importType, index});
  }
}

void Loader::parseFunctionSection(const shared_module_t &module) {
  (Printer::instance() << "parsing function section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(declaredFuncCount, uint32_t, module);
  for (uint32_t i = 0; i < declaredFuncCount; i++) {
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
  (Printer::instance() << "parsing table section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(tableCount, uint32_t, module);
  for (uint32_t i = 0; i < tableCount; i++) {
    // MVP: only support "anyfunc" (by default);
    const auto tableType = static_cast<ValueTypesCode>(Decoder::readUint8(module));
    if (tableType != ValueTypesCode::kFuncRef) {
      (Printer::instance() << "only support \"anyfunc\" type in table.\n").error();
    }

    // insert new element by placement-new && move;
    module->getTable()->emplace_back();
    auto *table = &module->getTable()->back();
    table->type = tableType;
    consumeTableParams(module, table);
  }
}

void Loader::parseMemorySection(const shared_module_t &module) {
  (Printer::instance() << "parsing memory section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(memeoryCount, uint32_t, module);

  // determine whether the memory has been initialized via "import";
  if (memeoryCount > 1 || module->getMemory() != nullptr) {
    (Printer::instance() << "only support one memory in MVP.\n").error();
  } else {
    consumeMemoryParams(module);
  }
}

void Loader::parseStartSection(const shared_module_t &module) {
  (Printer::instance() << "parsing start section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(startFuncIndex, uint32_t, module);

  // start function: no arguments or return value;
  const auto sig = module->getFunction(startFuncIndex)->sig;
  if (sig->paramsCount != 0 || sig->returnCount != 0) {
    (Printer::instance()
      << "the start function must not take any arguments or return value.\n").error();
  } else {
    module->startFuncIndex = startFuncIndex;
    module->hasValidStartFunc = true;
  }
}

void Loader::parseGlobalSection(const shared_module_t &module) {
  (Printer::instance() << "parsing global section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(globalCount, uint32_t, module);
  for (uint32_t i = 0; i < globalCount; i++) {
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
  (Printer::instance() << "parsing export section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(exportCount, uint32_t, module);
  for (uint32_t i = 0; i < exportCount; i++) {
    WRAP_UINT_FIELD(nameLen, uint32_t, module);
    const auto name = Decoder::decodeName(module, nameLen);
    const auto exportType = static_cast<ExternalTypesCode>(Decoder::readUint8(module));
    uint32_t index = 0;
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
          (Printer::instance() << "invalid memory index.\n").error();
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
        (Printer::instance() << "unkonwn export type.\n").error();
      }
    }
    module->getExport()->push_back({name, exportType, index});
  }
}

void Loader::parseCodeSection(const shared_module_t &module) {
  (Printer::instance() << "parsing code section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(bodyCount, uint32_t, module);
  for (uint32_t i = 0; i < bodyCount; i++) {
    WRAP_UINT_FIELD(bodySize, uint32_t, module);
    // update function body;
    const auto function = module->getFunction(module->importedFuncCount + i);
    // resolve locals;
    size_t step = 0;
    WRAP_UINT_FIELD_WITH_STEP(localEntryCount, uint32_t, module, &step);
    for (uint32_t j = 0; j < localEntryCount; j++) {
      WRAP_UINT_FIELD_WITH_STEP(localCount, uint32_t, module, &step);
      WRAP_UINT_FIELD_WITH_STEP(localTypeCode, uint8_t, module, &step);
      const auto type = static_cast<ValueFrameTypes>(localTypeCode);
      for (uint32_t k = 0; k < localCount; k++) {
        function->locals.push_back(type);
      }
    }
    function->code = module->getCurrentOffsetBuf();
    function->codeLen = bodySize - step;
    module->increaseBufOffset(function->codeLen);
  }
}

// initialize imported/internally-defined table;
void Loader::parseElementSection(const shared_module_t &module) {
  (Printer::instance() << "parsing element section.\n").debug();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
  WRAP_UINT_FIELD(elemCount, uint32_t, module);
  if (elemCount > 0 && module->getTable()->size() == 0) {
    (Printer::instance() << "no table found to apply the element section.\n").error();
  }
  for (uint32_t i = 0; i < elemCount; i++) {
    WRAP_UINT_FIELD(tableIndex, uint32_t, module);
    if (tableIndex != 0) {
      (Printer::instance() << "can only manipulate the default table in MVP.\n").error();
    }
    module->getElement()->emplace_back();
    auto *elem = &module->getElement()->back();
    elem->tableIndex = tableIndex;

    // deal with opcode;
    consumeInitExpr(module, &elem->init);

    // # of the element exprs;
    WRAP_UINT_FIELD(elemExprCount, uint32_t, module);
    for (uint32_t j = 0; j < elemExprCount; j++) {
      WRAP_UINT_FIELD(index, uint32_t, module);
      elem->entities.push_back(module->getFunction(index));
    }
  }
}

void Loader::parseDataSection(const shared_module_t &module) {
  (Printer::instance() << "parsing data section.\n").debug();
}

void Loader::skipKnownSection(uint8_t byte, const shared_module_t &module) {
  (Printer::instance().useHexFormat()
    << "unknown byte found: " << static_cast<int>(byte) << ".\n").error();
  [[maybe_unused]] WRAP_UINT_FIELD(payloadLen, uint32_t, module);
}
