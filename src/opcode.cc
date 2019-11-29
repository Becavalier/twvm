// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include <memory>
#include "src/types.h"
#include "src/opcode.h"
#include "src/macros.h"
#include "src/decoder.h"
#include "src/utils.h"

#define WRAP_FORWARD_INT_FIELD(keyName, type) \
  const auto keyName = Decoder::readVarInt<type>(executor->forward_());

using std::make_shared;

void OpCode::doUnreachable() {
  // trap;
  Utils::report("unreachable code!");
}

void OpCode::doBlock(shared_wasm_t &wasmIns, Executor *executor) {
  const auto labelStack = &wasmIns->stack->labelStack;
  const auto returnType = static_cast<ValueTypesCode>(Decoder::readUint8(executor->forward_()));
  labelStack->emplace(returnType, wasmIns->stack->valueStack.size());
  const auto topLabel = &labelStack->top();
  // find "end" entry;
  const auto topActivation = &wasmIns->stack->activationStack.top();
  size_t level = 0;
  executor->crawler(
    executor->pc + executor->innerOffset + 1,
    topActivation->pFuncIns->staticProto->codeLen - executor->innerOffset,
    [&level, &topLabel, &executor](WasmOpcode opcode, size_t offset) -> auto {
      switch (opcode) {
        case WasmOpcode::kOpcodeIf:
        case WasmOpcode::kOpcodeLoop:
        case WasmOpcode::kOpcodeBlock: {
          level++;
          break;
        }
        case WasmOpcode::kOpcodeEnd: {
          if (level == 0) {
            topLabel->end = make_shared<PosPtr>(executor->pc, executor->innerOffset + offset);
            return;
          } else {
            level--;
          }
          break;
        }
        default: break;
      }
  });
}

void OpCode::doLoop(shared_wasm_t &wasmIns, Executor *executor) {
  // "end" should be the start;
  
}

void OpCode::doIf(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doElse(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doEnd(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &currentLabelStackSize = wasmIns->stack->labelStack.size();
  const auto currentActivation = &wasmIns->stack->activationStack.top();
  const auto activationLabelStackHeight = currentActivation->getLabelStackHeight();
  const auto activationValueStackHeight = currentActivation->getValueStackHeight();
  if (currentLabelStackSize == activationLabelStackHeight) {
    // function end;
    const auto &funcProto = currentActivation->pFuncIns->staticProto;
    if (funcProto->sig->returnCount == (wasmIns->stack->valueStack.size() - activationValueStackHeight)) {
      const auto returnTypes = funcProto->sig->getReturnTypes();
      // pop operands from the stack;
      for (auto i = 0; i < returnTypes.size(); i++) {
        const auto topValue = &wasmIns->stack->valueStack.top();
        if (topValue->getValueType() == returnTypes.at(i)) {
          wasmIns->stack->tempValueStack.emplace(move(*topValue));
          wasmIns->stack->valueStack.pop();
        } else {
          Utils::report("return arity mismatch of the function!");
        }
      }
      for (auto i = 0; i < wasmIns->stack->tempValueStack.size(); i++) {
        wasmIns->stack->valueStack.emplace(move(wasmIns->stack->tempValueStack.top()));
        wasmIns->stack->tempValueStack.pop();
      }
    }
    // top-level function?
    if (wasmIns->stack->activationStack.size() == 1) {
      executor->switchStatus(false);
    }
  } else if (currentLabelStackSize > activationLabelStackHeight) {
    // control structure end;
  } else {
    Utils::report("invalide \"end(0xb)\" condition!");
  }
}

void OpCode::doBr(shared_wasm_t &wasmIns, Executor *executor) {
  WRAP_FORWARD_INT_FIELD(depth, int32_t);
  if (wasmIns->stack->labelStack.size() >= depth + 1) {

    cout << "br";
    std::cin.get();
  } else {
    Utils::report("invalid branching depth!");
  }
}

void OpCode::doBrIf(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = &wasmIns->stack->valueStack;
  const auto topVal = &valueStack->top();
  if (!topVal->isZero()) {
    doBr(wasmIns, executor);
  }
  valueStack->pop();
}

void OpCode::doBrTable(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doReturn(shared_wasm_t &wasmIns, Executor *executor) {
  cout << "return";
  std::cin.get();
}

void OpCode::doCall(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &modFuncs = wasmIns->module->funcs;
  WRAP_FORWARD_INT_FIELD(funcIndex, int32_t);
  if (funcIndex < modFuncs.size()) {
    const auto funcIns = modFuncs[funcIndex];
    executor->innerOffset = 0;
    executor->increaseCodeLen(funcIns->staticProto->codeLen);
    executor->pc = funcIns->staticProto->code - 1;
    // add an activation frame;
    const auto &stack = wasmIns->stack;
    const auto &wasmFunc = wasmIns->module->funcs.at(funcIndex);
    stack->activationStack.emplace(wasmFunc, stack->valueStack.size(), stack->labelStack.size());
    const auto activationFrame = &stack->activationStack.top();
    // initialize locals;
    for (const auto &paramType : wasmFunc->staticProto->sig->getParamTypes()) {
      const auto topVal = &wasmIns->stack->valueStack.top();
      if (topVal->getValueType() == paramType) {
        activationFrame->locals.emplace_back(move(*topVal));
        wasmIns->stack->valueStack.pop();
      }
    }
  } else {
    Utils::report("invalid function index to be called!");
  }
}

void OpCode::doLocalGet(shared_wasm_t &wasmIns, Executor *executor) {
  WRAP_FORWARD_INT_FIELD(localIndex, int32_t);
  const auto topActivation = &wasmIns->stack->activationStack.top();
  if (localIndex < topActivation->locals.size()) {
    // keep the "ValueFrames" in locals;
    wasmIns->stack->valueStack.push({&topActivation->locals.back()});
  }
}

void OpCode::doI32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i32 value onto the stack;
  wasmIns->stack->valueStack.push({
    Decoder::readVarInt<int32_t>(executor->forward_())});
}

void OpCode::doI64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i64 value onto the stack;
  wasmIns->stack->valueStack.push({
    Decoder::readVarInt<int64_t>(executor->forward_())});
}

void OpCode::doF32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f32 value onto the stack;
  wasmIns->stack->valueStack.push({
    Utils::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()))});
}

void OpCode::doF64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f64 value onto the stack;
  wasmIns->stack->valueStack.push({
    Utils::readUnalignedValue<double>(reinterpret_cast<uintptr_t>(executor->forward_()))});
}

// memory manipulation;
void OpCode::doI32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  // pop an i32 value from the stack (base address);
  const auto topVal = &wasmIns->stack->valueStack.top();
  if (topVal->getValueType() == ValueTypesCode::kI32) {
    const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
    // bitfields (alignment hint);
    WRAP_FORWARD_INT_FIELD(flags, int32_t);
    WRAP_FORWARD_INT_FIELD(offset, int32_t);
    const auto ea = topVal->toI32() + offset;
    if (ea + 4 <= mem->usedSize()) {
      wasmIns->stack->valueStack.push({mem->load<int32_t>(ea)});
    } else {
      Utils::report("memory access out of bound!");
    }
  } else {
    Utils::report("invalid stack on-top value type!");
  }
}

// numerical comparison;;
void OpCode::doI32GeS(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = &wasmIns->stack->valueStack;
  const auto tempValueStack = &wasmIns->stack->tempValueStack;
  if (valueStack->size() >= 2) {
    const auto topVal = &valueStack->top();
    tempValueStack->emplace(move(*topVal));
    valueStack->pop();
    if (valueStack->top() == tempValueStack->top()) {
      // put "i32.const 1" onto the stack;
      valueStack->top().resetValue<int32_t>(1);
    } else {
      // put "i32.const 0" onto the stack;
      valueStack->top().resetValue<int32_t>(0);
    }
    // discard the temp value;
    tempValueStack->pop();
  }
}

void OpCode::doI64GeS(shared_wasm_t &wasmIns, Executor *executor) {
  
}

void OpCode::doI32Add(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = &wasmIns->stack->valueStack;
  const auto tempValueStack = &wasmIns->stack->tempValueStack;
  if (valueStack->size() >= 2) {
    const auto topVal = &valueStack->top();
    tempValueStack->emplace(move(*topVal));
    valueStack->pop();
    const auto &firstVal = &valueStack->top();
    const auto &secondVal = &tempValueStack->top();
    if (firstVal->getValueType() == secondVal->getValueType()) {
      firstVal->resetValue<int32_t>(firstVal->toI32() + secondVal->toI32());
    }
    // discard the temp value;
    tempValueStack->pop();
  }
}

void OpCode::handle(shared_wasm_t wasmIns, WasmOpcode opcode, Executor *executor) {
  //std::cout << (int) opcode << std::endl;
  switch (opcode) {
    case WasmOpcode::kOpcodeUnreachable: { doUnreachable(); break; }
    case WasmOpcode::kOpcodeBlock: { doBlock(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeLoop: { doLoop(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeIf: { doIf(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeElse: { doElse(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeEnd: { doEnd(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeBr: { doBr(wasmIns, executor); break; } 
    case WasmOpcode::kOpcodeBrIf: { doBrIf(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeBrTable: { doBrTable(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeReturn: { doReturn(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeCall: { doCall(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeLocalGet: { doLocalGet(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI32Const: { doI32Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI64Const: { doI64Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeF32Const: { doF32Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeF64Const: { doF64Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI32LoadMem: { doI32LoadMem(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI32GeS: { doI32GeS(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI64GeS: { doI64GeS(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI32Add: { doI32Add(wasmIns, executor); break; }
    default: break;
  }
}
