// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include "src/types.h"
#include "src/opcode.h"
#include "src/include/macros.h"
#include "src/decoder.h"
#include "src/utils.h"

using std::forward;

#define ENABLE_DEBUG
#define WRAP_FORWARD_INT_FIELD(keyName, type) \
  const auto keyName = Decoder::readVarInt<type>(executor->forward_());

#ifdef ENABLE_DEBUG
  using std::hex;
  using std::showbase;
  #define INSPECT_STACK(opcodeName, wasmIns, executor) \
    const auto &vs = wasmIns->stack->valueStack; \
    const auto &ls = wasmIns->stack->labelStack; \
    const auto &as = wasmIns->stack->activationStack; \
    auto &printer = Printer::instance(); \
    stringstream line; \
    (printer << opcodeName << '\n').debug(); \
    line << "VS (values) | "; \
    for (uint32_t i = 0; i < vs->size(); i++) { \
      vs->at(i)->outputValue(line); \
      if (i < vs->size() - 1) { line << ", "; } \
    } \
    line << " <-"; \
    printer.makeLine(line); \
    line << "# of LS | " << ls->size(); \
    printer.makeLine(line); \
    line << "AS (locals) | "; \
    for (uint32_t i = 0; i < as->size(); i++) { \
      const auto &locals = as->at(i).locals; \
      const auto &localSize = locals.size(); \
      if (localSize == 0) { line << "void"; } else { \
        line << '['; \
        for (uint32_t j = 0; j < localSize; j ++) { \
          locals.at(j)->outputValue(line); \
          if (j < localSize - 1) { line << ", "; } \
        } \
        line << ']'; \
      } \
      if (i < as->size() - 1) { line << ", "; } \
    } \
    line << " <-"; \
    printer.makeLine(line); \
    line << "CP (i32) | "; \
    executor->int32ConstantPoolDebug(line); \
    printer.makeLine(line); \
    line << "CP (i64) | "; \
    executor->int64ConstantPoolDebug(line); \
    printer.makeLine(line); \
    line << "CP (f32) | "; \
    executor->floatConstantPoolDebug(line); \
    printer.makeLine(line); \
    line << "CP (f64) | "; \
    executor->doubleConstantPoolDebug(line); \
    printer.makeLine(line); \
    printer.printTableView();
#else
  #define INSPECT_STACK(...)
#endif

using std::make_shared;
using std::move;

void OpCode::doUnreachable() {
  // trap;
  (Printer::instance() << "unreachable code.\n").error();
}

void OpCode::doBlock(shared_wasm_t &wasmIns, Executor *executor) {
  cout << "block" << endl;
  const auto labelStack = wasmIns->stack->labelStack;
  // find immediate in cache first;
  const auto immediate = executor->uint8UseImmesCache(
    [&executor](size_t *step, uint8_t *immediate) -> auto {
      *immediate = Decoder::readUint8(executor->forward_());
      // it has already been forward by one "sizeof(uint8_t)", we need to subtract by one here;
      executor->innerOffset += ((*step = 1) - 1);
    });
  const auto returnType = static_cast<ValueTypesCode>(immediate);
  labelStack->emplace({returnType, wasmIns->stack->valueStack->size()});
  const auto topLabel = &labelStack->top();
  // find "end" entry;
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto endOffset = executor->int64UseMetaCache(
    OpcodeMeta::EndOffset, [&executor, &topActivation](int64_t *metaVal) -> auto {
      size_t level = 0;
      executor->crawler(
        executor->absAddr() + 1,
        topActivation->pFuncIns->staticProto->codeLen - executor->innerOffset,
        [&level, &metaVal](WasmOpcode opcode, size_t offset) -> auto {
          switch (opcode) {
            case WasmOpcode::kOpcodeIf:
            case WasmOpcode::kOpcodeLoop:
            case WasmOpcode::kOpcodeBlock: {
              level++;
              break;
            }
            case WasmOpcode::kOpcodeEnd: {
              if (level == 0) {
                *metaVal = offset;
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
    });
  topLabel->end = make_shared<PosPtr>(0, executor->pc, executor->innerOffset + endOffset - 1);

  INSPECT_STACK("block", wasmIns, executor);
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
      for (size_t i = 0; i < returnTypes.size(); i++) {
        const auto topValue = wasmIns->stack->valueStack->top(i);
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
    wasmIns->stack->activationStack->pop();
  } else if (currentLabelStackSize > activationLabelStackHeight) {
    // control structure end;
    wasmIns->stack->labelStack->pop();
  } else {
    (Printer::instance() << "invalide \"end(0xb)\" condition.\n").error();
  }
  INSPECT_STACK("end", wasmIns, executor);
}

void OpCode::doBr(shared_wasm_t &wasmIns, Executor *executor) {
  const auto depth = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto targetLabel = &wasmIns->stack->labelStack->top(depth);
  size_t skipTopVal = 0;
  if (targetLabel->getResultType() != ValueTypesCode::kVoid) {
    skipTopVal = 1;
  }

  if (wasmIns->stack->labelStack->size() >= depth + 1) {
    for (uint32_t i = 0; i < depth + 1; i++) {
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
        wasmIns->stack->labelStack->pop();
      }
    }
  } else {
    (Printer::instance() << "invalid branching depth.\n").error();
  }
  INSPECT_STACK("br", wasmIns, executor);
}

void OpCode::doBrIf(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = wasmIns->stack->valueStack;
  const auto isZero = valueStack->top()->isZero();
  valueStack->pop();
  if (!isZero) {
    doBr(wasmIns, executor);
  } else {
    // remove "depth" field;
    executor->innerOffset += Decoder::calcPassBytes(executor->absAddr() + 1);
  }
  INSPECT_STACK("br_if", wasmIns, executor);
}

void OpCode::doBrTable(shared_wasm_t &wasmIns, Executor *executor) {
}

void OpCode::doReturn(shared_wasm_t &wasmIns, Executor *executor) {
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto &leaveEntry = topActivation->leaveEntry;
  executor->pc = leaveEntry->pc;
  executor->innerOffset = leaveEntry->offset;
  // reset labels;
  for (size_t i = 0; i < wasmIns->stack->labelStack->size() - topActivation->getLabelStackHeight(); i++) {
    wasmIns->stack->labelStack->pop();
  }
  // reset activations;
  wasmIns->stack->activationStack->pop();
  INSPECT_STACK("return", wasmIns, executor);
}

void OpCode::doCall(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &modFuncs = wasmIns->module->funcs;
  const auto funcIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
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
    executor->pc = funcIns->code;
    // initialize locals;
    for (const auto &paramType : wasmFunc->staticProto->sig->getParamTypes()) {
      const auto topVal = wasmIns->stack->valueStack->top();
      if (topVal->getValueType() == paramType) {
        stack->activationStack->top().locals.emplace_back(topVal);
        wasmIns->stack->valueStack->pop();
      }
    }
  } else {
    (Printer::instance() << "invalid function index to be called.\n").error();
  }
  INSPECT_STACK("call", wasmIns, executor);
}

void OpCode::doLocalGet(shared_wasm_t &wasmIns, Executor *executor) {
  const auto localIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto topActivation = &wasmIns->stack->activationStack->top();
  if (localIndex < topActivation->locals.size()) {
    // keep the "ValueFrames" in locals;
    wasmIns->stack->valueStack->emplace(forward<ValueFrame*>(topActivation->locals[localIndex]));
  }
  INSPECT_STACK("local.get", wasmIns, executor);
}

void OpCode::doLocalSet(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("local.set", wasmIns, executor);
}

void OpCode::doLocalTee(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("local.tee", wasmIns, executor);
}

void OpCode::doGlobalGet(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("global.set", wasmIns, executor);
}

void OpCode::doGlobalSet(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("global.set", wasmIns, executor);
}

void OpCode::doI32Const(shared_wasm_t &wasmIns, Executor *executor) {
  cout << 123123123 << endl;
  // push an i32 value onto the stack;
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->int32UseImmesCache(
        [&executor](size_t *step, int32_t *immediate) -> auto {
          *immediate = Decoder::readVarInt<int32_t>(executor->forward_(), step);
          cout << executor->innerOffset << endl;
          executor->innerOffset += (*step - 1);
        })));
  INSPECT_STACK("i32.const", wasmIns, executor);
}

void OpCode::doI64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i64 value onto the stack;
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->int64UseImmesCache(
        [&executor](size_t *step, int64_t *immediate) -> auto {
          *immediate = Decoder::readVarInt<int64_t>(executor->forward_(), step);
          executor->innerOffset += (*step - 1);
        })));
  INSPECT_STACK("i64.const", wasmIns, executor);
}

void OpCode::doF32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f32 value onto the stack;
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->floatUseImmesCache(
        [&executor](size_t *step, float *immediate) -> auto {
          *immediate = Utils::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()));
          executor->innerOffset += ((*step = sizeof(float) / sizeof(uint8_t)) - 1);
        })));
  INSPECT_STACK("f32.const", wasmIns, executor);
}

void OpCode::doF64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f64 value onto the stack;
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->doubleUseImmesCache(
        [&executor](size_t *step, double *immediate) -> auto {
          *immediate = Utils::readUnalignedValue<double>(reinterpret_cast<uintptr_t>(executor->forward_()));
          executor->innerOffset += ((*step = sizeof(double) / sizeof(uint8_t)) - 1);
        })));
  INSPECT_STACK("f64.const", wasmIns, executor);
}

// operands: [baseAddr]; immes: [flags, offset]; return: [loadVal];
void OpCode::doI32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  // pop an i32 value from the stack (base address);
  const auto topVal = wasmIns->stack->valueStack->top();
  constexpr auto y = DEFAULT_ELEMENT_INDEX + 1;
  if (topVal->getValueType() == ValueTypesCode::kI32) {
    const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
    auto &v = executor->uint32UseMemargCache(
      [&executor](uint32_t *a, uint32_t *o, size_t *step) -> void {
        *a = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
        *o = Decoder::readVarUint<uint32_t>(executor->forward_() + *step - 1, step);
        executor->innerOffset += (*step - 2);
      });
    const auto ea = topVal->toI32() + v[y];
    // "sizeof(int32_t / 8)";
    if (ea + 4 <= mem->availableSize()) {
      // update directly;
      wasmIns->stack->valueStack->top() = executor->checkUpConstant(mem->load<int32_t>(ea));
    } else {
      (Printer::instance() << "memory access out of bound.\n").error();
    }
    // wasmIns->stack->valueStack->pop();
  } else {
    (Printer::instance() << "invalid stack on-top value type.\n").error();
  }
  INSPECT_STACK("i32.load", wasmIns, executor);
}

void OpCode::doI32LoadMem8S(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.load8_s", wasmIns, executor);
}

void OpCode::doI32LoadMem8U(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.load8_u", wasmIns, executor);
}

void OpCode::doI32LoadMem16S(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.load16_s", wasmIns, executor);
}

void OpCode::doI32LoadMem16U(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.load16_u", wasmIns, executor);
}

// operands: [storeVal, baseAddr]; immes: [flags, offset];
void OpCode::doI32StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  // pop an i32 value from the stack (value to be stored);
  auto topNVal = wasmIns->stack->valueStack->topN(2);
  constexpr auto x = DEFAULT_ELEMENT_INDEX;
  constexpr auto y = DEFAULT_ELEMENT_INDEX + 1;
  if ((*topNVal[x])->getValueType() == ValueTypesCode::kI32) {
    const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
    const auto storeVal = (*topNVal[x])->toI32();
    // retrive base address;
    if ((*topNVal[y])->getValueType() == ValueTypesCode::kI32) {
      auto &v = executor->uint32UseMemargCache(
        [&executor](uint32_t *a, uint32_t *o, size_t *step) -> void {
          *a = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
          *o = Decoder::readVarUint<uint32_t>(executor->forward_() + *step - 1, step);
          executor->innerOffset += (*step - 2);
        });
      const int32_t ea = (*topNVal[y])->toI32() + v[y];
      // "sizeof(int32_t / 8)";
      if (ea > 0 && ea + 4 <= mem->availableSize()) {
        mem->store<int32_t>(ea, storeVal);
      } else {
        (Printer::instance() << "memory access out of bound.\n").error();
      }
      wasmIns->stack->valueStack->popN(2);
    }
  }
  INSPECT_STACK("i32.store", wasmIns, executor);
}

void OpCode::doI32StoreMem8(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.store8", wasmIns, executor);
}

void OpCode::doI32StoreMem16(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i32.store16", wasmIns, executor);
}

void OpCode::doI64LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load", wasmIns, executor);
}

void OpCode::doI64LoadMem8S(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load8_s", wasmIns, executor);
}

void OpCode::doI64LoadMem8U(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load8_u", wasmIns, executor);
}

void OpCode::doI64LoadMem16S(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load16_s", wasmIns, executor);
}

void OpCode::doI64LoadMem16U(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load16_u", wasmIns, executor);
}

void OpCode::doI64LoadMem32S(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load32_s", wasmIns, executor);
}

void OpCode::doI64LoadMem32U(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.load32_u", wasmIns, executor);
}

void OpCode::doI64StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.store", wasmIns, executor);
}

void OpCode::doI64StoreMem8(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.store8", wasmIns, executor);
}

void OpCode::doI64StoreMem16(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.store16", wasmIns, executor);
}

void OpCode::doI64StoreMem32(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("i64.store32", wasmIns, executor);
}

void OpCode::doF32StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("f32.store", wasmIns, executor);
}

void OpCode::doF32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("f32.load", wasmIns, executor);
}

void OpCode::doF64StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("f64.store", wasmIns, executor);
}

void OpCode::doF64LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("f64.load", wasmIns, executor);
}

// numerical comparison;;
void OpCode::doI32GeS(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = wasmIns->stack->valueStack;
  if (valueStack->size() < 2) {
    (Printer::instance() << "operands not enough to be consumed.\n").error();
  }
  const auto operands = valueStack->topN(2);
  const auto &y = *operands.at(0);
  const auto &x = *operands.at(1);
  if (y->getValueType() == ValueTypesCode::kI32 && x->getValueType() == ValueTypesCode::kI32) {
    const int32_t result = x->toI32() >= y->toI32() ? 1 : 0;
    valueStack->pop();
    valueStack->top() = executor->checkUpConstant(result);
  } else {
    (Printer::instance() << "wrong operands type.\n").error();
  }
  INSPECT_STACK("i32.ge_s", wasmIns, executor);
}

void OpCode::doI64GeS(shared_wasm_t &wasmIns, Executor *executor) {
}

void OpCode::doI32Add(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = wasmIns->stack->valueStack;
  if (valueStack->size() < 2) {
    (Printer::instance() << "operands not enough to be consumed.\n").error();
  }
  const auto operands = valueStack->topN(2);
  const auto &y = *operands.at(0);
  const auto &x = *operands.at(1);
  if (y->getValueType() == ValueTypesCode::kI32 && x->getValueType() == ValueTypesCode::kI32) {
    const int32_t result = x->toI32() + y->toI32();
    valueStack->pop();
    valueStack->top() = executor->checkUpConstant(result);
  } else {
    (Printer::instance() << "wrong operands type.\n").error();
  }
  INSPECT_STACK("i32.add", wasmIns, executor);
}
