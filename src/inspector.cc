// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <sstream>
#include "src/utilities.h"
#include "src/inspector.h"

#define OUT \
  Utilities::reportDebug()

using std::cout;
using std::endl;
using std::hex;
using std::showbase;
using std::stringstream;

void Inspector::inspect(shared_ptr<WasmInstance> ins) {
  stringstream line;
  // set up display format;
  line << hex << showbase;
  const auto printer = Utilities::getPrinter();

  OUT << endl;
  OUT << hex << showbase << "- [INSPECTOR PHASE] -" << endl;
  
  // WasmFunctionSig;
  OUT << "# Signatures (" << ins->module->types.size() << "): " << endl;
  for (const auto &type : ins->module->types) {
    auto i = 0;
    const auto reps = type->reps;
    line << "[";
    line << type->index << ": ";
    for (; i < type->paramsCount; i++) {
      line << static_cast<int>(reps[i]) << ' ';
    }
    for (; i < (type->paramsCount + type->returnCount); i++) {
      line << "-> " << static_cast<int>(reps[i]);
    }
    line << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmFunction;
  OUT << "# Functions (" << ins->module->funcs.size() << "): " <<  endl;
  for (const auto &func : ins->module->funcs) {
    line << "[";
    line << "sig_index " << func->type->index << " | " << "code_size " << func->code.size() << " B]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmTable;
  OUT << "# Tables (" << ins->module->tables.size() << "): " << endl;
  for (const auto &table : ins->module->tables) {
    line << "[";
    line << "max_table_size " << table->maxTableSize << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmMemory;
  OUT << "# Memories (" << ins->module->memories.size() << "): " << endl;
  for (const auto &memory : ins->module->memories) {
    line << "[";
    line << "memory_size " << (memory->size() * WASM_PAGE_SIZE / 1024) << " kib]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmGlobal;
  OUT << "# Globals (" << ins->module->globals.size() << "): " << endl;
  for (const auto &global : ins->module->globals) {
    line << "[";
    line << "global_type " << static_cast<int>(global->type) << " | ";
    line << "mutability " << global->mutability << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmExport;
  OUT << "# Exports (" << ins->module->exports.size() << "): " << endl;
  for (const auto &_export : ins->module->exports) {
    line << "[";
    line << "export_name \"" << _export.name << "\" | ";
    line << "export_type " << static_cast<int>(_export.type) << " | ";
    line << "index " << _export.index << "]";
    printer->feedLine(line);
  }
  printer->printTableView();
}
