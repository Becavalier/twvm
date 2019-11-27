// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include "src/opcode.h"
#include "src/macros.h"
#include "src/decoder.h"
#include "src/utils.h"

void OpCode::doUnreachable() {
  // trap;
  Utils::report("unreachable code!");
}

void OpCode::doEnd(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  const auto &currentLabelStackSize = wasmIns->stack->labelStack.size();
  const auto currentActivation = &wasmIns->stack->activationStack.top();
  const auto activationLabelStackHeight = currentActivation->getLabelStackHeight();
  const auto activationValueStackHeight = currentActivation->getValueStackHeight();
  if (currentLabelStackSize == activationLabelStackHeight) {
    // function end;
    if (currentActivation->pFuncIns->sig->returnCount == (wasmIns->stack->valueStack.size() - activationValueStackHeight)) {
      const auto returnTypes = currentActivation->pFuncIns->sig->getReturnTypes();
      // pop operands from the stack;
      for (auto i = 0; i < returnTypes.size(); i++) {
        const auto topValue = &wasmIns->stack->valueStack.top();
        if (topValue->getValueType() == returnTypes.at(i)) {
          wasmIns->stack->returnValueStack.emplace(move(*topValue));
          wasmIns->stack->valueStack.pop();
        } else {
          Utils::report("return arity mismatch of the function!");
        }
      }
      for (auto i = 0; i < wasmIns->stack->returnValueStack.size(); i++) {
        wasmIns->stack->valueStack.emplace(move(wasmIns->stack->returnValueStack.top()));
      }
    }
  } else if (currentLabelStackSize > activationLabelStackHeight) {
    // control structure end;
  } else {
    Utils::report("invalide \"end(0xb)\" condition!");
  }
}

void OpCode::doI32Const(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  // push an i32 value onto the stack;
  wasmIns->stack->valueStack.push({
    static_cast<int32_t>(Decoder::readVarInt<int32_t>(executor->forward_()))});
}

void OpCode::doI64Const(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  // push an i64 value onto the stack;
  wasmIns->stack->valueStack.push({
    static_cast<int64_t>(Decoder::readVarInt<int64_t>(executor->forward_()))});
}

void OpCode::doF32Const(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  // push a f32 value onto the stack;
  wasmIns->stack->valueStack.push({
    Utils::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()))});
}

void OpCode::doF64Const(shared_ptr<WasmInstance> &wasmIns, Executor *executor) {
  // push a f64 value onto the stack;
  wasmIns->stack->valueStack.push({
    Utils::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()))});
}

void OpCode::handle(shared_ptr<WasmInstance> wasmIns, WasmOpcode opcode, Executor *executor) {
  std::cout << (int) opcode << std::endl;
  switch (opcode) {
    case WasmOpcode::kOpcodeUnreachable: {
      doUnreachable();
      break;
    }
    case WasmOpcode::kOpcodeEnd: { doEnd(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI32Const: { doI32Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeI64Const: { doI64Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeF32Const: { doF32Const(wasmIns, executor); break; }
    case WasmOpcode::kOpcodeF64Const: { doF64Const(wasmIns, executor); break; }
    default:
      break;
  }
}
