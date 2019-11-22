// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <sstream>
#include "src/utilities.h"
#include "src/inspector.h"
#include "src/macros.h"

using std::cout;
using std::endl;
using std::hex;
using std::showbase;
using std::stringstream;

void Inspector::inspect(shared_ptr<WasmInstance> wasmIns) {
  stringstream line;
  // set up display format;
  line << hex << showbase;
  const auto printer = Utilities::getPrinter();

  DEBUG_OUT() << endl;
  DEBUG_OUT() << hex << showbase << "- [INSPECTING PHASE] -" << endl;
  
  // WasmFunctionSig;
  DEBUG_OUT() << "# Signatures (" << wasmIns->module->types.size() << "): " << endl;
  for (const auto &type : wasmIns->module->types) {
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
  DEBUG_OUT() << "# Functions (" << wasmIns->module->funcs.size() << "): " <<  endl;
  for (const auto &func : wasmIns->module->funcs) {
    line << "[";
    line << "sig_index " << func->type->index << " | " << "code_size " << func->code.size() << " B]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmTable;
  DEBUG_OUT() << "# Tables (" << wasmIns->module->tables.size() << "): " << endl;
  for (const auto &table : wasmIns->module->tables) {
    line << "[";
    line << "max_table_size " << table->maxTableSize << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmMemory;
  DEBUG_OUT() << "# Memories (" << wasmIns->module->memories.size() << "): " << endl;
  for (const auto &memory : wasmIns->module->memories) {
    line << "[";
    line << "memory_size " << (memory->size() * WASM_PAGE_SIZE / 1024) << " kib]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmGlobal;
  DEBUG_OUT() << "# Globals (" << wasmIns->module->globals.size() << "): " << endl;
  for (const auto &global : wasmIns->module->globals) {
    line << "[";
    line << "global_type " << static_cast<int>(global->type) << " | ";
    line << "mutability " << global->mutability << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmExport;
  DEBUG_OUT() << "# Exports (" << wasmIns->module->exports.size() << "): " << endl;
  for (const auto &_export : wasmIns->module->exports) {
    line << "[";
    line << "export_name \"" << _export.name << "\" | ";
    line << "export_type " << static_cast<int>(_export.type) << " | ";
    line << "index " << _export.index << "]";
    printer->feedLine(line);
  }
  printer->printTableView();
}
