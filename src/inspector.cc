// Copyright 2019 YHSPY. All rights reserved.
#include <iostream>
#include "src/utilities.h"
#include "src/inspector.h"

#define OUT \
  Utilities::reportDebug()

using std::cout;
using std::endl;
using std::hex;

bool Inspector::inspect(shared_ptr<WasmInstance> ins) {
  OUT << hex << "- [INSPECTOR PHASE] -" << endl;
  
  // WasmFunctionSig;
  OUT << "count of Signatures: " << ins->module->types.size() << endl;
  for (const auto &type : ins->module->types) {
    const auto reps = type->reps;
    auto i = 0;
    OUT << "( " << type->index << ": ";
    for (; i < type->paramsCount; i++) {
      cout << "[param " << static_cast<int>(reps[i]) << "] ";
    }
    for (; i < type->paramsCount + type->returnCount; i++) {
      cout << "-> [result " << static_cast<int>(reps[i]) << "] ";
    }
    cout << ")" << endl;
  }

  // WasmFunction;
  OUT << "count of Functions: " << ins->module->funcs.size() << endl;
  for (const auto func : ins->module->funcs) {
    OUT << "- ";
    cout << "[sig_index " << func->type->index << "] ";
    cout << "[code_size " << func->code.size() << "] ";
    cout << "-" << endl;
  }

  return true;
}
