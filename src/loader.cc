// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <array>
#include <stdexcept>
#include <iostream>
#include "src/loader.h"
#include "src/include/constants.h"
#include "src/utils.h"
#include "src/opcode.h"

vector<uchar_t> Loader::buf;
ifstream* Loader::reader;
uint32_t Loader::byteCounter = 0;
size_t Loader::currentReaderOffset = 0;

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
    if (WRAP_BUF_UINT32() != kWasmMagicWord) {
      (Printer::instance() << "invalid wasm magic word, expect 0x6d736100.\n").error();
    }
    // offset;
    if (WRAP_BUF_UINT32() != kWasmVersion) {
      (Printer::instance() << "invalid wasm version, expect 0x01.\n").error();
    }
  } else {
    (Printer::instance() << "can not reading file.\n").error();
  }

  // parsing start;
  return parse(wasmModule);
}

shared_module_t Loader::parse(const shared_module_t &module) {
  parseSection(module);
  return module;
}

void Loader::parseSection(const shared_module_t &module) {
  while (reader->peek() != EOF) {
    const auto sectionCode = WRAP_READER_VARUINT(uint8_t);
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
        (Printer::instance() << "unidentified section found.\n").error();
        break;
      }
    }
  }
  buf.clear();
}

void Loader::parseUnkownSection(uint8_t sectionCode, const shared_module_t &module) {
  // buzzle;
  Printer::instance().useHexFormat()
    << "customized section code: " << static_cast<int>(sectionCode) << ".\n";
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
}

void Loader::parseTypeSection(const shared_module_t &module) {
  (Printer::instance() << "parsing type section.\n").debug();
  // payload length;
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  // self-counting from "buf";
  const auto entryCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < entryCount; i++) {
    if (static_cast<ValueTypesCode>(WRAP_BUF_UINT8()) == ValueTypesCode::kFunc) {
      module->getFunctionSig()->emplace_back();
      const auto sig = &module->getFunctionSig()->back();
      const auto paramsCount = WRAP_BUF_VARUINT(uint32_t);
      for (uint32_t j = 0; j < paramsCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(WRAP_BUF_UINT8()));
      }
      const auto returnCount = WRAP_BUF_VARUINT(uint8_t);
      for (uint32_t j = 0; j < returnCount; j++) {
        sig->reps.push_back(static_cast<ValueTypesCode>(WRAP_BUF_UINT8()));
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
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto importCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < importCount; i++) {
    // set module name;
    const auto moduleName = WRAP_BUF_STRING(WRAP_BUF_VARUINT(uint32_t));

    // set field name;
    const auto fieldName = WRAP_BUF_STRING(WRAP_BUF_VARUINT(uint32_t));
    const auto importType = static_cast<ExternalTypesCode>(WRAP_BUF_UINT8());
    uint32_t index = 0;
    switch (importType) {
      case ExternalTypesCode::kExternalFunction: {
        const auto sigIndex = WRAP_BUF_VARUINT(uint32_t);
        const auto sig = module->getFunctionSig(sigIndex);
        index = module->getFunction()->size();
        module->importedFuncCount++;
        module->getFunction()->push_back({sig, index, sigIndex, {}, {}, 0, true, false});
        break;
      }
      case ExternalTypesCode::kExternalTable: {
        // MVP: only support "anyfunc" (by default);
        const auto tableType = static_cast<ValueTypesCode>(WRAP_BUF_UINT8());
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
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto declaredFuncCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < declaredFuncCount; i++) {
    module->getFunction()->emplace_back();
    const auto func = &module->getFunction()->back();
    // indices: uint32_t;
    const auto sigIndex = WRAP_BUF_VARUINT(uint32_t);
    const auto sig = module->getFunctionSig(sigIndex);
    const auto funcIndex = module->getFunction()->size();
    func->sig = sig;
    func->sigIndex = sigIndex;
    func->funcIndex = funcIndex;
  }
}

void Loader::parseTableSection(const shared_module_t &module) {
  (Printer::instance() << "parsing table section.\n").debug();
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto tableCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < tableCount; i++) {
    // MVP: only support "anyfunc" (by default);
    const auto tableType = static_cast<ValueTypesCode>(WRAP_BUF_UINT8());
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
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto memeoryCount = WRAP_BUF_VARUINT(uint32_t);
  // determine whether the memory has been initialized via "import";
  if (memeoryCount > 1 || module->getMemory() != nullptr) {
    (Printer::instance() << "only support one memory in MVP.\n").error();
  } else {
    consumeMemoryParams(module);
  }
}

void Loader::parseStartSection(const shared_module_t &module) {
  (Printer::instance() << "parsing start section.\n").debug();
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto startFuncIndex = WRAP_BUF_VARUINT(uint32_t);
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
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto globalCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < globalCount; i++) {
    const auto contentType = static_cast<ValueTypesCode>(WRAP_BUF_UINT8());
    const auto mutability = WRAP_BUF_UINT8() == kWasmTrue;

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
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto exportCount = WRAP_BUF_VARUINT(uint32_t);
  for (uint32_t i = 0; i < exportCount; i++) {
    const auto nameLen = WRAP_BUF_VARUINT(uint32_t);
    const auto name = WRAP_BUF_STRING(nameLen);
    const auto exportType = static_cast<ExternalTypesCode>(WRAP_BUF_UINT8());
    uint32_t index = 0;
    switch (exportType) {
      case ExternalTypesCode::kExternalFunction: {
        index = WRAP_BUF_VARUINT(uint32_t);
        const auto funcIns = module->getFunction(index);
        funcIns->exported = true;
        break;
      }
      case ExternalTypesCode::kExternalTable: {
        index = WRAP_BUF_VARUINT(uint32_t);
        const auto table = module->getTable(index);
        if (table != nullptr) {
          table->exported = true;
        }
        break;
      }
      case ExternalTypesCode::kExternalMemory: {
        index = WRAP_BUF_VARUINT(uint32_t);
        const auto mem = module->getMemory();
        if (index != 0 || mem == nullptr) {
          (Printer::instance() << "invalid memory index.\n").error();
        } else {
          mem->exported = true;
        }
        break;
      }
      case ExternalTypesCode::kExternalGlobal: {
        index = WRAP_BUF_VARUINT(uint32_t);
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
  [[maybe_unused]] const auto payloadLen = WRAP_READER_VARUINT(uint32_t);
  const auto bodyCount = WRAP_READER_VARUINT(uint32_t);
  for (uint32_t i = 0; i < bodyCount; i++) {
    const auto bodySize = WRAP_READER_VARUINT(uint32_t);
    retrieveBytes(bodySize);
    // update function body;
    const auto function = module->getFunction(module->importedFuncCount + i);
    // resolve locals;
    const auto localEntryCount = WRAP_BUF_VARUINT(uint32_t);
    for (uint32_t j = 0; j < localEntryCount; j++) {
      const auto localCount = WRAP_BUF_VARUINT(uint32_t);
      const auto localTypeCode = WRAP_BUF_VARUINT(uint8_t);
      const auto type = static_cast<ValueFrameTypes>(localTypeCode);
      for (uint32_t k = 0; k < localCount; k++) {
        function->locals.push_back(type);
      }
    }
    function->codeLen = bodySize - currentReaderOffset;
    for (size_t j = 0; j < function->codeLen; j++) {
      const auto opcode = WRAP_BUF_UINT8();
      // threading;
      cout << sizeof(uint8_t) << endl;
      cout << reinterpret_cast<uintptr_t>(&OpCode::doBrIf) << endl;
      function->code.push_back(opcode);
    }
  }
}

// initialize imported/internally-defined table;
void Loader::parseElementSection(const shared_module_t &module) {
  (Printer::instance() << "parsing element section.\n").debug();
  retrieveBytes(WRAP_READER_VARUINT(uint32_t));
  const auto elemCount = WRAP_BUF_VARUINT(uint32_t);
  if (elemCount > 0 && module->getTable()->size() == 0) {
    (Printer::instance() << "no table found to apply the element section.\n").error();
  }
  for (uint32_t i = 0; i < elemCount; i++) {
    const auto tableIndex = WRAP_BUF_VARUINT(uint32_t);
    if (tableIndex != 0) {
      (Printer::instance() << "can only manipulate the default table in MVP.\n").error();
    }
    module->getElement()->emplace_back();
    auto *elem = &module->getElement()->back();
    elem->tableIndex = tableIndex;

    // deal with opcode;
    consumeInitExpr(module, &elem->init);

    // # of the element exprs;
    const auto elemExprCount = WRAP_BUF_VARUINT(uint32_t);
    for (uint32_t j = 0; j < elemExprCount; j++) {
      elem->entities.push_back(module->getFunction(WRAP_BUF_VARUINT(uint32_t)));
    }
  }
}

void Loader::parseDataSection(const shared_module_t &module) {
  (Printer::instance() << "parsing data section.\n").debug();
}
