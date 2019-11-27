// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "src/cmdline.h"
#include "src/utils.h"
#include "src/inspector.h"
#include "src/macros.h"

using std::cout;
using std::endl;
using std::hex;
using std::showbase;
using std::stringstream;
using std::to_string;

void Inspector::inspect(shared_ptr<WasmInstance> wasmIns) {
  if (!CommandLine::isDebugMode) {
    return;
  }

  stringstream line;
  // set up display format;
  line << hex << showbase;
  const auto printer = Utils::getPrinter();

  Utils::debug();
  Utils::debug(string("- [INSPECTING PHASE] -"), true);

  // WasmFunctionSig;
  const auto &typeSize = wasmIns->module->types.size();
  Utils::debug({
    "# Signatures (",
    to_string(wasmIns->module->types.size()),
    "): ", (typeSize == 0 ? "N/A" : string())});
  for (const auto &type : wasmIns->module->types) {
    auto i = 0;
    const auto reps = type->reps;
    line << "[";
    line << type->index << ": ";
    for (; i < type->paramsCount; i++) {
      line << static_cast<int>(reps[i]) << ' ';
    }
    if (type->paramsCount == 0) { line << "void"; }
    const auto slotCount = type->paramsCount + type->returnCount;
    for (; i < slotCount; i++) {
      line << "-> " << static_cast<int>(reps[i]);
    }
    if (type->returnCount == 0) { line << "void"; }
    line << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmFunction;
  const auto &funcSize = wasmIns->module->funcs.size();
  Utils::debug({
    "# Functions (",
    to_string(wasmIns->module->funcs.size()), "): ",
    (funcSize == 0 ? "N/A" : string())});
  for (const auto &func : wasmIns->module->funcs) {
    line << "[";
    line << "sig_index " << func->type->index << " | "
      << "locals(" << func->staticProto->locals.size() << ")";
    for (const auto &type : func->staticProto->locals) {
      line << static_cast<int>(type) << endl;
    }
    line << " | " << "code_size " << func->code.size() << " B]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmTable;
  const auto &tableSize = wasmIns->module->tables.size();
  Utils::debug({
    "# Tables (",
    to_string(tableSize), "): ",
    (tableSize == 0 ? "N/A" : string())});
  for (const auto &table : wasmIns->module->tables) {
    line << "[";
    line << "max_table_size " << table->maxTableSize << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmMemory;
  const auto &memorySize = wasmIns->module->memories.size();
  Utils::debug({
    "# Memories (",
    to_string(memorySize), "): ",
    (memorySize == 0 ? "N/A" : string())});
  for (const auto &memory : wasmIns->module->memories) {
    line << "[";
    line << "memory_size " << (memory->size() * WASM_PAGE_SIZE / 1024) << " kib]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmGlobal;
  const auto &globalSize = wasmIns->module->globals.size();
  Utils::debug({
    "# Globals (",
    to_string(globalSize), "): ",
    (globalSize == 0 ? "N/A" : string())});
  for (const auto &global : wasmIns->module->globals) {
    line << "[";
    line << "global_type " << static_cast<int>(global->type) << " | ";
    line << "mutability " << global->mutability << "]";
    printer->feedLine(line);
  }
  printer->printTableView();

  // WasmExport;
  const auto &exportSize = wasmIns->module->exports.size();
  Utils::debug({
    "# Exports (",
    to_string(exportSize), "): ",
    (exportSize == 0 ? "N/A" : string())});
  for (const auto &_export : wasmIns->module->exports) {
    line << "[";
    line << "export_name \"" << _export.name << "\" | ";
    line << "export_type " << static_cast<int>(_export.type) << " | ";
    line << "index " << _export.index << "]";
    printer->feedLine(line);
  }
  printer->printTableView();
}
