// Copyright 2019 YHSPY. All rights reserved.
#include "src/executor.h"
#include "src/include/macros.h"
#include "src/opcode.h"
#include "src/decoder.h"
#include "src/utils.h"

const int Executor::execute(shared_ptr<WasmInstance> wasmIns) {
  (Printer::instance() << '\n').debug();
  (Printer::instance() << "- [EXECUTING PHASE] -\n").debug();

  pc = wasmIns->startPoint->pc;
  contextIndex = wasmIns->startPoint->index;

  while (true) {
    if (!runningStatus) {
      // verify running reuslt by the state of final stack;
      return wasmIns->stack->checkStackState(wasmIns->startEntry);
    }

    uintptr_t r;
    memcpy(&r, pc->data() + (++innerOffset), sizeof(r));
    reinterpret_cast<handlerSig*>(r)(wasmIns, this);
    innerOffset+=sizeof(r);

/*
    const WasmOpcode opcode = static_cast<WasmOpcode>(pc->at(++innerOffset));
    // run;
    OpCode::handle(wasmIns, opcode, this);
    */
  }
  return 0;
}

const void Executor::crawler(
    const uchar_t* buf, size_t length, const function<bool(WasmOpcode, size_t)> &callback) {
  // eat every opcode and immediates;
  size_t offset = 0;
  while (offset != length) {
    const auto opcode = static_cast<WasmOpcode>(*(buf + offset++));
    switch (opcode) {
      case WasmOpcode::kOpcodeI32Const:
      case WasmOpcode::kOpcodeI64Const:
      case WasmOpcode::kOpcodeBrIf:
      case WasmOpcode::kOpcodeIf:
      case WasmOpcode::kOpcodeCall:
      case WasmOpcode::kOpcodeLocalGet:
      case WasmOpcode::kOpcodeLocalSet:
      case WasmOpcode::kOpcodeLocalTee:
      case WasmOpcode::kOpcodeGlobalGet:
      case WasmOpcode::kOpcodeGlobalSet: {
        offset += Decoder::calcPassBytes(buf + offset);
        break;
      }
      // "memory_immediate";
      case WasmOpcode::kOpcodeF32LoadMem:
      case WasmOpcode::kOpcodeF64LoadMem:
      case WasmOpcode::kOpcodeI32LoadMem:
      case WasmOpcode::kOpcodeI64LoadMem:
      case WasmOpcode::kOpcodeI32LoadMem8S:
      case WasmOpcode::kOpcodeI32LoadMem8U:
      case WasmOpcode::kOpcodeI64LoadMem8S:
      case WasmOpcode::kOpcodeI64LoadMem8U:
      case WasmOpcode::kOpcodeI32LoadMem16S:
      case WasmOpcode::kOpcodeI32LoadMem16U:
      case WasmOpcode::kOpcodeI64LoadMem16S:
      case WasmOpcode::kOpcodeI64LoadMem16U:
      case WasmOpcode::kOpcodeI64LoadMem32S:
      case WasmOpcode::kOpcodeI64LoadMem32U:
      case WasmOpcode::kOpcodeI32StoreMem:
      case WasmOpcode::kOpcodeI64StoreMem:
      case WasmOpcode::kOpcodeF32StoreMem:
      case WasmOpcode::kOpcodeF64StoreMem:
      case WasmOpcode::kOpcodeI32StoreMem8:
      case WasmOpcode::kOpcodeI64StoreMem8:
      case WasmOpcode::kOpcodeI32StoreMem16:
      case WasmOpcode::kOpcodeI64StoreMem16:
      case WasmOpcode::kOpcodeI64StoreMem32: {
        offset += Decoder::calcPassBytes(buf + offset, 2);
        break;
      }
      case WasmOpcode::kOpcodeF64Const: {
        offset += 2;
        break;
      }
      case WasmOpcode::kOpcodeBrTable: {
        //
        break;
      }
      default: break;
    }
    if (callback && callback(opcode, offset)) {
      return;
    }
  }
}
