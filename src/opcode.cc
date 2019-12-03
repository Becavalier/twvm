// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include <memory>
#include "src/types.h"
#include "src/opcode.h"
#include "src/macros.h"
#include "src/decoder.h"
#include "src/utils.h"

// #define ENABLE_DEBUG
#define WRAP_FORWARD_INT_FIELD(keyName, type) \
  const auto keyName = Decoder::readVarInt<type>(executor->forward_());
#ifdef ENABLE_DEBUG
  using std::hex;
  using std::showbase;
  #define RESPECT_STACK(opcodeName, wasmIns, executor) \
    const auto &vs = wasmIns->stack->valueStack; \
    const auto &ls = wasmIns->stack->labelStack; \
    const auto &as = wasmIns->stack->activationStack; \
    auto &printer = Printer::instance(); \
    stringstream line; \
    (printer << opcodeName).debug(); \
    cout << hex << showbase << '(' << static_cast<int>(executor->getCurrentOpcode()) << "):" << endl; \
    line << "VS (values) | "; \
    for (auto i = 0; i < vs->size(); i++) { \
      vs->at(i).outputValue(line); \
      if (i < vs->size() - 1) { line << ", "; } \
    } \
    line << " <-"; \
    printer.makeLine(line); \
    line << "# of LS | " << ls->size(); \
    printer.makeLine(line); \
    line << "AS (locals) | "; \
    for (auto i = 0; i < as->size(); i++) { \
      const auto &locals = as->at(i).locals; \
      const auto &localSize = locals.size(); \
      if (localSize == 0) { line << "void"; } else { \
        line << '['; \
        for (auto j = 0; j < localSize; j ++) { \
          locals.at(j).outputValue(line); \
          if (j < localSize - 1) { line << ", "; } \
        } \
        line << ']'; \
      } \
      if (i < as->size() - 1) { line << ", "; } \
    } \
    line << " <-"; \
    printer.makeLine(line); \
    printer.printTableView();
#else
  #define RESPECT_STACK(...)
#endif

#define GET_TWO_OPERANDS() \
  const auto valueStack = wasmIns->stack->valueStack; \
  if (valueStack->size() < 2) { \
    (Printer::instance() << "operands not enough to be consumed.\n").error(); \
  } \
  const auto operands = valueStack->topN(2); \
  const auto &c2 = operands.at(1)->toI32(); \
  const auto &c1 = operands.at(0)->toI32(); \
  valueStack->popN();

using std::make_shared;

void OpCode::doUnreachable() {
  // trap;
  (Printer::instance() << "unreachable code.\n").error();
}

void OpCode::doBlock(shared_wasm_t &wasmIns, Executor *executor) {
  const auto labelStack = wasmIns->stack->labelStack;
  // find immediate in cache first;
  const auto immediate = executor->uint8SetCache([&executor](...) -> auto {
    return Decoder::readUint8(executor->forward_());
  });
  const auto returnType = static_cast<ValueTypesCode>(immediate);
  labelStack->emplace({returnType, wasmIns->stack->valueStack->size()});
  const auto topLabel = &labelStack->top();
  // find "end" entry;
  const auto topActivation = &wasmIns->stack->activationStack->top();
  size_t level = 0;
  executor->crawler(
    executor->absAddr() + 1,
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
            topLabel->end = make_shared<PosPtr>(0, executor->pc, executor->innerOffset + offset - 1);
            return true;
          } else {
            level--;
          }
          break;
        }
        default: break;
      }
      return false;
  });
  RESPECT_STACK("block", wasmIns, executor);
}

void OpCode::doLoop(shared_wasm_t &wasmIns, Executor *executor) {
  // "end" should be the start;
  
}

void OpCode::doIf(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doElse(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doEnd(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &currentLabelStackSize = wasmIns->stack->labelStack->size();
  const auto &currentActivation = &wasmIns->stack->activationStack->top();
  const auto activationLabelStackHeight = currentActivation->getLabelStackHeight();
  const auto activationValueStackHeight = currentActivation->getValueStackHeight();
  if (currentLabelStackSize == activationLabelStackHeight) {
    // function end;
    const auto &funcProto = currentActivation->pFuncIns->staticProto;
    if (funcProto->sig->returnCount == (wasmIns->stack->valueStack->size() - activationValueStackHeight)) {
      const auto returnTypes = funcProto->sig->getReturnTypes();
      // check the type of return operands;
      for (auto i = 0; i < returnTypes.size(); i++) {
        const auto topValue = &wasmIns->stack->valueStack->top(i);
        if (topValue->getValueType() != returnTypes.at(i)) {
          (Printer::instance() << "return arity mismatch of the function.\n").error();
        }
      }
      // top-level function?
      if (wasmIns->stack->activationStack->size() == 1) {
        executor->switchStatus(false);
      } else {
        // go-on here;
        const auto &leaveEntry = currentActivation->leaveEntry;
        executor->pc = leaveEntry->pc;
        executor->innerOffset = leaveEntry->offset;
      }
    }
    wasmIns->stack->activationStack->popN();
  } else if (currentLabelStackSize > activationLabelStackHeight) {
    // control structure end;
    wasmIns->stack->labelStack->popN();
  } else {
    (Printer::instance() << "invalide \"end(0xb)\" condition.\n").error();
  }
  RESPECT_STACK("end", wasmIns, executor);
}

void OpCode::doBr(shared_wasm_t &wasmIns, Executor *executor, bool innerCall) {
  WRAP_FORWARD_INT_FIELD(depth, int32_t);
  const auto targetLabel = &wasmIns->stack->labelStack->top(depth);
  size_t skipTopVal = 0;
  if (targetLabel->getResultType() != ValueTypesCode::kVoid) {
    skipTopVal = 1;
  }
    
  if (wasmIns->stack->labelStack->size() >= depth + 1) {
    for (auto i = 0; i < depth + 1; i++) {
      // only leave the last "LabelFrame";
      if (i == depth) {
        // last round (end / start), redirect pointer;
        const auto topLabel = &wasmIns->stack->labelStack->top();
        executor->pc = topLabel->end->pc;
        executor->innerOffset = topLabel->end->offset;
      } else {
        const auto topLabel = &wasmIns->stack->labelStack->top();
        const auto stackHeightDiff = wasmIns->stack->valueStack->size() - topLabel->getValueStackHeight() - skipTopVal;
        wasmIns->stack->valueStack->erase(skipTopVal, stackHeightDiff);
        wasmIns->stack->labelStack->popN();
      }
    }
  } else {
    (Printer::instance() << "invalid branching depth.\n").error();
  }
  RESPECT_STACK((innerCall ? "br_if: br" : "br"), wasmIns, executor);
}

void OpCode::doBrIf(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = wasmIns->stack->valueStack;
  const auto isZero = valueStack->top().isZero();
  valueStack->popN();
  if (!isZero) {
    doBr(wasmIns, executor, true);
  } else {
    // remove "depth" field;
    executor->innerOffset += Decoder::calcPassBytes(executor->absAddr() + 1);
  }
  RESPECT_STACK("br_if", wasmIns, executor);
}

void OpCode::doBrTable(shared_wasm_t &wasmIns, Executor *executor) {

}

void OpCode::doReturn(shared_wasm_t &wasmIns, Executor *executor) {
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto &leaveEntry = topActivation->leaveEntry;
  executor->pc = leaveEntry->pc;
  executor->innerOffset = leaveEntry->offset;
  // reset labels;
  for (auto i = 0; i < wasmIns->stack->labelStack->size() - topActivation->getLabelStackHeight(); i++) {
    wasmIns->stack->labelStack->popN();
  }
  // reset activations;
  wasmIns->stack->activationStack->popN();
  RESPECT_STACK("return", wasmIns, executor);
}

void OpCode::doCall(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &modFuncs = wasmIns->module->funcs;
  WRAP_FORWARD_INT_FIELD(funcIndex, int32_t);
  if (funcIndex < modFuncs.size()) {
    // add an activation frame;
    const auto &stack = wasmIns->stack;
    const auto &wasmFunc = wasmIns->module->funcs.at(funcIndex);
    const auto &paramCount = wasmFunc->staticProto->sig->paramsCount;
    if (stack->valueStack->size() < paramCount) {
      (Printer::instance() << "operands not enough to be consumed.\n").error();
    }
    stack->activationStack->emplace({
      wasmFunc,
      // subtract the count of locals for initialization;
      stack->valueStack->size() - paramCount,
      stack->labelStack->size(),
      make_shared<PosPtr>(funcIndex, executor->pc, executor->innerOffset)});
    // redirect;
    const auto funcIns = modFuncs[funcIndex];
    executor->innerOffset = -1;
    executor->pc = &funcIns->code;
    // initialize locals;
    for (const auto &paramType : wasmFunc->staticProto->sig->getParamTypes()) {
      const auto topVal = &wasmIns->stack->valueStack->top();
      if (topVal->getValueType() == paramType) {
        stack->activationStack->top().locals.emplace_back(move(*topVal));
        wasmIns->stack->valueStack->popN();
      }
    }
  } else {
    (Printer::instance() << "invalid function index to be called.\n").error();
  }
  RESPECT_STACK("call", wasmIns, executor);
}

void OpCode::doLocalGet(shared_wasm_t &wasmIns, Executor *executor) {
  WRAP_FORWARD_INT_FIELD(localIndex, int32_t);
  const auto topActivation = &wasmIns->stack->activationStack->top();
  if (localIndex < topActivation->locals.size()) {
    // keep the "ValueFrames" in locals;
    wasmIns->stack->valueStack->push({&topActivation->locals.at(localIndex)});
  }
  RESPECT_STACK("get_local", wasmIns, executor);
}

void OpCode::doI32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i32 value onto the stack;
  wasmIns->stack->valueStack->push({
    executor->int32SetCache([&executor](size_t *step) -> auto {
      return Decoder::readVarInt<int32_t>(executor->forward_(), step);
    })});
  RESPECT_STACK("i32.const", wasmIns, executor);
}

void OpCode::doI64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i64 value onto the stack;
  wasmIns->stack->valueStack->push({
    Decoder::readVarInt<int64_t>(executor->forward_())});
  RESPECT_STACK("i64.const", wasmIns, executor);
}

void OpCode::doF32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f32 value onto the stack;
  wasmIns->stack->valueStack->push({
    Utils::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()))});
  RESPECT_STACK("f32.const", wasmIns, executor);
}

void OpCode::doF64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f64 value onto the stack;
  wasmIns->stack->valueStack->push({
    Utils::readUnalignedValue<double>(reinterpret_cast<uintptr_t>(executor->forward_()))});
  RESPECT_STACK("f64.const", wasmIns, executor);
}

// memory manipulation;
void OpCode::doI32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  // pop an i32 value from the stack (base address);
  const auto topVal = &wasmIns->stack->valueStack->top();
  if (topVal->getValueType() == ValueTypesCode::kI32) {
    const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
    // bitfields (alignment hint);
    WRAP_FORWARD_INT_FIELD(flags, int32_t);
    WRAP_FORWARD_INT_FIELD(offset, int32_t);
    const auto ea = topVal->toI32() + offset;
    if (ea + 4 <= mem->usedSize()) {
      wasmIns->stack->valueStack->push({mem->load<int32_t>(ea)});
    } else {
      (Printer::instance() << "memory access out of bound.\n").error();
    }
  } else {
    (Printer::instance() << "invalid stack on-top value type.\n").error();
  }
  RESPECT_STACK("i32.load", wasmIns, executor);
}

// numerical comparison;;
void OpCode::doI32GeS(shared_wasm_t &wasmIns, Executor *executor) {
  GET_TWO_OPERANDS();
  valueStack->top().resetValue<int32_t>(c1 >= c2 ? 1 : 0);
  RESPECT_STACK("i32.ge_s", wasmIns, executor);
}

void OpCode::doI64GeS(shared_wasm_t &wasmIns, Executor *executor) {
  
}

void OpCode::doI32Add(shared_wasm_t &wasmIns, Executor *executor) {
  GET_TWO_OPERANDS();
  valueStack->top().resetValue<int32_t>(c1 + c2);
  RESPECT_STACK("i32.add", wasmIns, executor);
}

void OpCode::handle(shared_wasm_t wasmIns, WasmOpcode opcode, Executor *executor) {
  // std::cout << (int) opcode << std::endl;
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
