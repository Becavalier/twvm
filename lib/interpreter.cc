// Copyright 2019 YHSPY. All rights reserved.
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <cmath>
#include <string>
#include <limits>
#include "lib/type.h"
#include "lib/decoder.h"
#include "lib/utility.h"
#include "lib/interpreter.h"
#include "lib/common/bits.h"
#include "lib/common/errors.h"
#include "lib/common/macros.h"
#include "lib/structures/struct-stack.h"

#define DECLARE_BASIC_BINOP_METHOD(name, action, operandType, resultType, op, label) \
  void Interpreter::do##name##action(shared_wasm_t &wasmIns, Executor *executor) { \
    retrieveDoubleRTVals<operandType>(wasmIns, executor, \
      [&executor]( \
        const shared_ptr<Stack::ValueFrameStack> &valueStack, \
        const operandType x, \
        const operandType y) -> void { \
        valueStack->pop(); \
        valueStack->top() = executor->checkUpConstant( \
          static_cast<resultType>(x op y)); \
      }); \
    INSPECT_STACK(#label, wasmIns, executor); \
}

#define ITERATE_SIMPLE_BINOP(V) \
  V(I32, Mul, int32_t, int32_t, *, i32.mul) \
  V(I32, Add, int32_t, int32_t, +, i32.add) \
  V(I32, Sub, int32_t, int32_t, -, i32.sub) \
  V(I32, And, int32_t, int32_t, &, i32.and) \
  V(I32, Or, int32_t, int32_t, |, i32.or) \
  V(I32, Xor, int32_t, int32_t, ^, i32.xor) \
  V(I32, Eq, int32_t, int32_t, ==, i32.eq) \
  V(I32, Ne, int32_t, int32_t, !=, i32.ne) \
  V(I32, LtU, uint32_t, uint32_t, <, i32.ltu) \
  V(I32, LeU, uint32_t, uint32_t, <=, i32.leu) \
  V(I32, GtU, uint32_t, uint32_t, >, i32.gtu) \
  V(I32, GeU, uint32_t, uint32_t, >=, i32.geu) \
  V(I32, LtS, int32_t, int32_t, <, i32.lts) \
  V(I32, LeS, int32_t, int32_t, <=, i32.les) \
  V(I32, GtS, int32_t, int32_t, >, i32.gts) \
  V(I32, GeS, int32_t, int32_t, >=, i32.ges) \
  V(I64, Mul, int64_t, int64_t, *, i64.mul) \
  V(I64, Add, int64_t, int64_t, +, i64.add) \
  V(I64, Sub, int64_t, int64_t, -, i64.sub) \
  V(I64, And, int64_t, int64_t, &, i64.and) \
  V(I64, Or, int64_t, int64_t, |, i64.or) \
  V(I64, Xor, int64_t, int64_t, ^, i64.xor) \
  V(I64, Eq, int64_t, int32_t, ==, i64.eq) \
  V(I64, Ne, int64_t, int32_t, !=, i64.ne) \
  V(I64, LtU, uint64_t, uint32_t, <, i64.ltu) \
  V(I64, LeU, uint64_t, uint32_t, <=, i64.leu) \
  V(I64, GtU, uint64_t, uint32_t, >, i64.gtu) \
  V(I64, GeU, uint64_t, uint32_t, >=, i64.geu) \
  V(I64, LtS, int64_t, int32_t, <, i64.lts) \
  V(I64, LeS, int64_t, int32_t, <=, i64.les) \
  V(I64, GtS, int64_t, int32_t, >, i64.gts) \
  V(I64, GeS, int64_t, int32_t, >=, i64.ges) \
  V(F32, Mul, float, float, *, f32.mul) \
  V(F32, Add, float, float, +, f32.add) \
  V(F32, Sub, float, float, -, f32.sub) \
  V(F32, Div, float, float, /, f32.div) \
  V(F32, Eq, float, int32_t, ==, f32.eq) \
  V(F32, Ne, float, int32_t, !=, f32.ne) \
  V(F32, Lt, float, int32_t, <, f32.lt) \
  V(F32, Le, float, int32_t, <=, f32.le) \
  V(F32, Gt, float, int32_t, >, f32.gt) \
  V(F32, Ge, float, int32_t, >=, f32.ge) \
  V(F64, Mul, double, double, *, f64.mul) \
  V(F64, Add, double, double, +, f64.add) \
  V(F64, Sub, double, double, -, f64.sub) \
  V(F64, Div, double, double, /, f64.div) \
  V(F64, Eq, double, int32_t, ==, f64.eq) \
  V(F64, Ne, double, int32_t, !=, f64.ne) \
  V(F64, Lt, double, int32_t, <, f64.lt) \
  V(F64, Le, double, int32_t, <=, f64.le) \
  V(F64, Gt, double, int32_t, >, f64.gt) \
  V(F64, Ge, double, int32_t, >=, f64.ge)

using std::forward;
using std::floor;
using ::ceilf;
using ::ceill;
using ::sqrtf;
using std::sqrt;
using std::abs;
using std::nearbyintf;
using std::copysign;
using std::copysignf;
using std::numeric_limits;
using std::trunc;
using std::string;
using std::ostringstream;

#define WRAP_FORWARD_INT_FIELD(keyName, type) \
  const auto keyName = Decoder::readVarInt<type>(executor->forward_());

// use C-Standard "NDEBUG" macro.
#ifndef NDEBUG
  using std::hex;
  using std::showbase;
  #define INSPECT_STACK(opcodeName, wasmIns, executor) \
    debug(opcodeName, wasmIns, executor)
#else
  #define INSPECT_STACK(...)
#endif

using std::make_shared;
using std::move;

template <typename T>
void Interpreter::retrieveDoubleRTVals(
  shared_wasm_t &wasmIns,
  Executor *executor,
  const function<void(
    const shared_ptr<Stack::ValueFrameStack> &valueStack,
    const T x, const T y)> &handler) {
  const auto valueStack = wasmIns->stack->valueStack;
  if (valueStack->size() < 2) {
    Printer::instance().error(Errors::RT_OPERANDS_NOT_ENOUGH);
  }
  const auto operands = valueStack->topN(2);
  const auto &y = *operands.at(DEFAULT_ELEMENT_INDEX);
  const auto &x = *operands.at(DEFAULT_ELEMENT_INDEX + 1);
  if constexpr (is_same<T, int32_t>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kI32 && x->getGenericType() == ValueTypesCode::kI32)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toI32(), y->toI32());
    }
  } else if constexpr (is_same<T, uint32_t>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kI32 && x->getGenericType() == ValueTypesCode::kI32)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toU32(), y->toU32());
    }
  } else if constexpr (is_same<T, int64_t>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kI64 && x->getGenericType() == ValueTypesCode::kI64)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toI64(), y->toI64());
    }
  } else if constexpr (is_same<T, uint64_t>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kI64 && x->getGenericType() == ValueTypesCode::kI64)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toU64(), y->toU64());
    }
  } else if constexpr (is_same<T, float>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kF32 && x->getGenericType() == ValueTypesCode::kF32)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toF32(), y->toF32());
    }
  } else if constexpr (is_same<T, double>::value) {
    if (!(y->getGenericType() == ValueTypesCode::kF64 && x->getGenericType() == ValueTypesCode::kF64)) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH); 
    } else {
      handler(valueStack, x->toF64(), y->toF64());
    }
  }
}

template <typename T>
void Interpreter::retrieveSingleRTVal(
  shared_wasm_t &wasmIns,
  Executor *executor,
  const function<void(
    const shared_ptr<Stack::ValueFrameStack> &valueStack,
    const T x)> &handler) {
  const auto valueStack = wasmIns->stack->valueStack;
  const auto &x = valueStack->top();
  if constexpr (is_same<T, int32_t>::value) {
    if (x->getGenericType() != ValueTypesCode::kI32) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toI32());
    }
  } else if constexpr (is_same<T, uint32_t>::value) {
    if (x->getGenericType() != ValueTypesCode::kI32) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toU32());
    }
  } else if constexpr (is_same<T, int64_t>::value) {
    if (x->getGenericType() != ValueTypesCode::kI64) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toI64());
    }
  } else if constexpr (is_same<T, uint64_t>::value) {
    if (x->getGenericType() != ValueTypesCode::kI64) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toU64());
    }
  } else if constexpr (is_same<T, float>::value) {
    if (x->getGenericType() != ValueTypesCode::kF32) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toF32());
    }
  } else if constexpr (is_same<T, double>::value) {
    if (x->getGenericType() != ValueTypesCode::kF64) {
      Printer::instance().error(Errors::RT_OPERANDS_TYPE_MISMATCH);
    } else {
      handler(valueStack, x->toF64());
    }
  }
}

template <typename T>
void Interpreter::storeMemarg(
  shared_wasm_t &wasmIns,
  Executor *executor,
  const function<void(const int32_t, WasmMemoryInstance *const &mem, const T)> &handler) {
    const auto valueStack = wasmIns->stack->valueStack;
    if (valueStack->size() < 2) {
      Printer::instance().error(Errors::RT_OPERANDS_NOT_ENOUGH);
    }
    // pop an i32 value from the stack (value to be stored).
    auto topNVal = valueStack->topN(2);
    constexpr auto x = DEFAULT_ELEMENT_INDEX;
    constexpr auto y = DEFAULT_ELEMENT_INDEX + 1;
    const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
    T storeVal = T{};
    // i32 / i64 / f32 / f64.
    const auto storedFrame = *topNVal[x];
    if constexpr (is_same<T, int32_t>::value) {
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTI32Value) {
        storeVal = storedFrame->toI32();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    } else if constexpr (is_same<T, uint32_t>::value) {
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTU32Value) {
        storeVal = storedFrame->toU32();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    } else if constexpr (is_same<T, uint32_t>::value) {
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTI64Value) {
        storeVal = storedFrame->toI64();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    } else if constexpr (is_same<T, uint64_t>::value) {
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTU64Value) {
        storeVal = storedFrame->toU64();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    } else if constexpr (is_same<T, float>::value) {
      static_assert(sizeof(float) == 4, "insufficient size of the type.");
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTF32Value) {
        storeVal = storedFrame->toF32();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    } else if constexpr (is_same<T, double>::value) {
      static_assert(sizeof(double) == 8, "insufficient size of the type.");
      if (storedFrame->getRTValueType() == ValueFrameTypes::kRTF64Value) {
        storeVal = storedFrame->toF64();
      } else {
        Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
      }
    }
    // retrive base address.
    const auto baseFrame = *topNVal[y];
    if (baseFrame->getRTValueType() == ValueFrameTypes::kRTI32Value) {
      auto &v = executor->uint32UseMemargCache(
        [&executor](uint32_t *a, uint32_t *o, size_t *step) -> void {
          *a = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
          *o = Decoder::readVarUint<uint32_t>(executor->forward_() + *step - 1, step);
          executor->innerOffset += (*step - 2);
        });
      const int32_t ea = baseFrame->toI32() + v[y];
      // "sizeof(int8_t / 8)".
      if (ea >= 0) {
        handler(ea, mem, storeVal);
      } else {
        Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
      }
      wasmIns->stack->valueStack->popN(2);
    }
  }

void Interpreter::retrieveMemarg(
  shared_wasm_t &wasmIns,
  Executor *executor,
  const function<void(const int32_t, WasmMemoryInstance *const &mem)> &handler) {
    // pop an i32 value from the stack (base address).
    const auto topVal = wasmIns->stack->valueStack->top();
    constexpr auto y = DEFAULT_ELEMENT_INDEX + 1;
    if (topVal->getRTValueType() == ValueFrameTypes::kRTI32Value) {
      const auto &mem = wasmIns->module->memories[DEFAULT_ELEMENT_INDEX];
      auto &v = executor->uint32UseMemargCache(
        [&executor](uint32_t *a, uint32_t *o, size_t *step) -> void {
          *a = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
          *o = Decoder::readVarUint<uint32_t>(executor->forward_() + *step - 1, step);
          executor->innerOffset += (*step - 2);
        });
      const auto ea = topVal->toI32() + v[y];
      // "sizeof(int8_t / 8)".
      if (ea >= 0) {
        handler(ea, mem);
      } else {
        Printer::instance().error(Errors::RT_MEM_ACCESS_OOB);
      }
    } else {
      Printer::instance().error(Errors::RT_INVALID_STACK_VAL);
    }
  }

void debug(string opcodeName, const shared_wasm_t &wasmIns, Executor *executor) {
  const auto &valueStack = wasmIns->stack->valueStack;
  const auto &labelStack = wasmIns->stack->labelStack;
  const auto &activationStack = wasmIns->stack->activationStack;
  auto &printer = Printer::instance();
  ostringstream line;
  (printer << opcodeName << '\n').debug();
  // "ValueFrame".
  line << "VS (values) | ";
  for (uint32_t i = 0; i < valueStack->size(); ++i) {
    valueStack->at(i)->outputValue(line);
    if (i < valueStack->size() - 1) { line << ", "; }
  }
  line << " <-";
  printer.makeLine(line);
  // "LabelFrame".
  line << "# of LS | " << labelStack->size();
  printer.makeLine(line);
  // "ActivationFrame".
  line << "AS (locals) | ";
  for (uint32_t i = 0; i < activationStack->size(); ++i) {
    const auto &locals = activationStack->at(i).locals;
    const auto &localSize = locals.size();
    if (localSize == 0) {
      line << "void";
    } else {
      line << '[';
      for (uint32_t j = 0; j < localSize; ++j) {
        const auto local = locals.at(j);
        if (local != nullptr) { 
          local->outputValue(line); 
        } else {
          line << "n/a"; 
        }
        if (j < localSize - 1) { line << ", "; }
      }
      line << ']';
    }
    if (i < activationStack->size() - 1) { line << ", "; }
  }
  line << " <-";
  printer.makeLine(line);
  // constant pool.
  #define DECLARE_CP_INSPECT_METHOD(name, ...) \
    line << "CP (" << #name << ") | "; \
    executor->name##ConstantPoolDebug(line); \
    printer.makeLine(line);
  ITERATE_OPERANDS_VALUE_TYPES(DECLARE_CP_INSPECT_METHOD)
  printer.printTableView();
}

ITERATE_SIMPLE_BINOP(DECLARE_BASIC_BINOP_METHOD)

void Interpreter::doUnreachable(shared_wasm_t &wasmIns, Executor *executor) {
  // trap immediately.
  Printer::instance().error(Errors::RT_UNREACHABLE_CODE);
}

void Interpreter::doBlock(shared_wasm_t &wasmIns, Executor *executor) {
  const auto labelStack = wasmIns->stack->labelStack;
  // find immediate in cache first.
  const auto immediate = executor->uint8UseImmesCache(
    [&executor](size_t *step, uint8_t *immediate) -> auto {
      *immediate = Decoder::readUint8(executor->forward_());
      // it has already been forward by one "sizeof(uint8_t)", we need to subtract by one here.
      executor->innerOffset += ((*step = 1) - 1);
    });
  const auto returnType = static_cast<ValueTypesCode>(immediate);
  labelStack->emplace({returnType, wasmIns->stack->valueStack->size()});
  const auto topLabel = &labelStack->top();
  // find "end" entry.
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto endOffset = executor->int64UseMetaCache(
    OpcodeMeta::EndOffset, [&executor, &topActivation](int64_t *metaVal) -> auto {
      size_t level = 0;
      executor->crawler(
        executor->absAddr() + 1,
        topActivation->pFuncIns->code->size() - executor->innerOffset,
        [&level, &metaVal](WasmOpCode opcode, size_t offset) -> auto {
          switch (opcode) {
            case WasmOpCode::kOpcodeIf:
            case WasmOpCode::kOpcodeLoop:
            case WasmOpCode::kOpcodeBlock: {
              level += 1;
              break;
            }
            case WasmOpCode::kOpcodeEnd: {
              if (level == 0) {
                // move the pointer back to the opcode.
#if defined(OPT_DCT)
                *metaVal = offset - 8;
#else
                *metaVal = offset;
#endif
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

void Interpreter::doLoop(shared_wasm_t &wasmIns, Executor *executor) {
  // "end" should be the start.
}

void Interpreter::doIf(shared_wasm_t &wasmIns, Executor *executor) {
}

void Interpreter::doElse(shared_wasm_t &wasmIns, Executor *executor) {
}

void Interpreter::doEnd(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &currentLabelStackSize = wasmIns->stack->labelStack->size();
  const auto &currentActivation = &wasmIns->stack->activationStack->top();
  const auto activationLabelStackHeight = currentActivation->labelStackHeight;
  const auto activationValueStackHeight = currentActivation->valueStackHeight;
  if (currentLabelStackSize == activationLabelStackHeight) {
    // function end.
    const auto &funcProto = currentActivation->pFuncIns->staticProto;
    if (funcProto->sig->returnCount == (wasmIns->stack->valueStack->size() - activationValueStackHeight)) {
      const auto returnTypes = funcProto->sig->getReturnTypes();
      // check the type of return operands.
      for (size_t i = 0; i < returnTypes.size(); ++i) {
        const auto topValue = wasmIns->stack->valueStack->top(i);
        if (topValue->getGenericType() != returnTypes.at(i)) {
          Printer::instance().error(Errors::RT_ARITY_MISMATCH);
        }
      }
      // top-level function?
      if (wasmIns->stack->activationStack->size() == 1) {
        executor->switchStatus(false);
      } else {
        // go-on here.
        const auto &leaveEntry = currentActivation->leaveEntry;
        executor->pc = leaveEntry->pc;
        executor->innerOffset = leaveEntry->offset;
      }
    }
    wasmIns->stack->activationStack->pop();
  } else if (currentLabelStackSize > activationLabelStackHeight) {
    // control structure end.
    wasmIns->stack->labelStack->pop();
  } else {
    Printer::instance().error(Errors::RT_INVALID_END);
  }
  INSPECT_STACK("end", wasmIns, executor);
}

void Interpreter::doBr(shared_wasm_t &wasmIns, Executor *executor) {
  const auto depth = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto targetLabel = &wasmIns->stack->labelStack->top(depth);
  size_t skipTopVal = 0;
  if (targetLabel->resultType != ValueTypesCode::kVoid) {
    skipTopVal = 1;
  }

  if (wasmIns->stack->labelStack->size() >= depth + 1) {
    for (uint32_t i = 0; i < depth + 1; ++i) {
      // only leave the last "LabelFrame".
      if (i == depth) {
        // last round (end / start), redirect pointer.
        const auto topLabel = &wasmIns->stack->labelStack->top();
        executor->pc = topLabel->end->pc;
        executor->innerOffset = topLabel->end->offset;
      } else {
        const auto topLabel = &wasmIns->stack->labelStack->top();
        const auto stackHeightDiff = wasmIns->stack->valueStack->size() - topLabel->valueStackHeight - skipTopVal;
        wasmIns->stack->valueStack->erase(skipTopVal, stackHeightDiff);
        wasmIns->stack->labelStack->pop();
      }
    }
  } else {
    Printer::instance().error(Errors::RT_INVALID_BRANCH_DEPTH);
  }
  INSPECT_STACK("br", wasmIns, executor);
}

void Interpreter::doBrIf(shared_wasm_t &wasmIns, Executor *executor) {
  const auto valueStack = wasmIns->stack->valueStack;
  const auto isZero = valueStack->top()->isZero();
  valueStack->pop();
  if (!isZero) {
    doBr(wasmIns, executor);
  } else {
    // remove "depth" field.
    executor->innerOffset += Decoder::calcPassBytes(executor->absAddr() + 1);
  }
  INSPECT_STACK("br_if", wasmIns, executor);
}

void Interpreter::doBrTable(shared_wasm_t &wasmIns, Executor *executor) {
}

void Interpreter::doReturn(shared_wasm_t &wasmIns, Executor *executor) {
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto &leaveEntry = topActivation->leaveEntry;
  executor->pc = leaveEntry->pc;
  executor->innerOffset = leaveEntry->offset;
  // reset labels.
  for (size_t i = 0; i < wasmIns->stack->labelStack->size() - topActivation->labelStackHeight; ++i) {
    wasmIns->stack->labelStack->pop();
  }
  // reset activations.
  wasmIns->stack->activationStack->pop();
  INSPECT_STACK("return", wasmIns, executor);
}

void Interpreter::doCall(shared_wasm_t &wasmIns, Executor *executor) {
  const auto &modFuncs = wasmIns->module->funcs;
  const auto funcIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  if (funcIndex < modFuncs.size()) {
    // add an activation frame.
    const auto &stack = wasmIns->stack;
    const auto &wasmFunc = wasmIns->module->funcs.at(funcIndex);
    const auto &paramCount = wasmFunc->staticProto->sig->paramsCount;
    if (stack->valueStack->size() < paramCount) {
      Printer::instance().error(Errors::RT_OPERANDS_NOT_ENOUGH);
    }
    stack->activationStack->emplace({
      wasmFunc,
      // subtract the count of locals for initialization.
      stack->valueStack->size() - paramCount,
      stack->labelStack->size(),
      make_shared<PosPtr>(funcIndex, executor->pc, executor->innerOffset)});
    // redirect.
    const auto funcIns = modFuncs[funcIndex];
    executor->innerOffset = -1;
    executor->pc = funcIns->code;
    // initialize locals.
    for (const auto &paramType : wasmFunc->staticProto->sig->getParamTypes()) {
      const auto topVal = wasmIns->stack->valueStack->top();
      if (topVal->getGenericType() == paramType) {
        stack->activationStack->top().locals.emplace_back(topVal);
        wasmIns->stack->valueStack->pop();
      }
    }
  } else {
    Printer::instance().error(Errors::RT_INVALID_FUNC_INDEX);
  }
  INSPECT_STACK("call", wasmIns, executor);
}

void Interpreter::doLocalGet(shared_wasm_t &wasmIns, Executor *executor) {
  const auto localIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto topActivation = &wasmIns->stack->activationStack->top();
  if (localIndex < topActivation->locals.size()) {
    // keep the "ValueFrames" in locals.
    wasmIns->stack->valueStack->emplace(forward<ValueFrame*>(topActivation->locals[localIndex]));
  }
  INSPECT_STACK("local.get", wasmIns, executor);
}

void Interpreter::doLocalSet(shared_wasm_t &wasmIns, Executor *executor) {
  const auto localIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto topVal = wasmIns->stack->valueStack->top();
  topActivation->locals[localIndex] = topVal;
  // remove the top "ValueFrame".
  wasmIns->stack->valueStack->pop();
  INSPECT_STACK("local.set", wasmIns, executor);
}

void Interpreter::doLocalTee(shared_wasm_t &wasmIns, Executor *executor) {
  // write a local and return(keep) the same value.
  const auto localIndex = executor->uint32UseImmesCache(
    [&executor](size_t *step, uint32_t *immediate) -> auto {
      *immediate = Decoder::readVarUint<uint32_t>(executor->forward_(), step);
      executor->innerOffset += (*step - 1);
    });
  const auto topActivation = &wasmIns->stack->activationStack->top();
  const auto topVal = wasmIns->stack->valueStack->top();
  topActivation->locals[localIndex] = topVal;
  INSPECT_STACK("local.tee", wasmIns, executor);
}

void Interpreter::doGlobalGet(shared_wasm_t &wasmIns, Executor *executor) {
  
  INSPECT_STACK("global.get", wasmIns, executor);
}

void Interpreter::doGlobalSet(shared_wasm_t &wasmIns, Executor *executor) {
  INSPECT_STACK("global.set", wasmIns, executor);
}

void Interpreter::doI32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i32 value onto the stack.
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->int32UseImmesCache(
        [&executor](size_t *step, int32_t *immediate) -> auto {
          *immediate = Decoder::readVarInt<int32_t>(executor->forward_(), step);
          executor->innerOffset += (*step - 1);
        })));
  INSPECT_STACK("i32.const", wasmIns, executor);
}

void Interpreter::doI64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push an i64 value onto the stack.
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->int64UseImmesCache(
        [&executor](size_t *step, int64_t *immediate) -> auto {
          *immediate = Decoder::readVarInt<int64_t>(executor->forward_(), step);
          executor->innerOffset += (*step - 1);
        })));
  INSPECT_STACK("i64.const", wasmIns, executor);
}

void Interpreter::doF32Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f32 value onto the stack.
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->floatUseImmesCache(
        [&executor](size_t *step, float *immediate) -> auto {
          *immediate = Utility::readUnalignedValue<float>(reinterpret_cast<uintptr_t>(executor->forward_()));
          executor->innerOffset += ((*step = sizeof(float) / sizeof(uint8_t)) - 1);
        })));
  INSPECT_STACK("f32.const", wasmIns, executor);
}

void Interpreter::doF64Const(shared_wasm_t &wasmIns, Executor *executor) {
  // push a f64 value onto the stack.
  wasmIns->stack->valueStack->emplace(
    executor->checkUpConstant(
      executor->doubleUseImmesCache(
        [&executor](size_t *step, double *immediate) -> auto {
          *immediate = Utility::readUnalignedValue<double>(reinterpret_cast<uintptr_t>(executor->forward_()));
          executor->innerOffset += ((*step = sizeof(double) / sizeof(uint8_t)) - 1);
        })));
  INSPECT_STACK("f64.const", wasmIns, executor);
}

// operands: [baseAddr]; immes: [flags, offset]; return: [loadVal].
void Interpreter::doI32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(mem->load<int32_t>(ea));
      });
  INSPECT_STACK("i32.load", wasmIns, executor);
}

void Interpreter::doI32LoadMem8S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int32_t>(mem->load<int8_t>(ea)));
      });
  INSPECT_STACK("i32.load8_s", wasmIns, executor);
}

void Interpreter::doI32LoadMem8U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int32_t>(mem->load<uint8_t>(ea)));
      });
  INSPECT_STACK("i32.load8_u", wasmIns, executor);
}

void Interpreter::doI32LoadMem16S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int32_t>(mem->load<int16_t>(ea)));
      });
  INSPECT_STACK("i32.load16_s", wasmIns, executor);
}

void Interpreter::doI32LoadMem16U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int32_t>(mem->load<uint16_t>(ea)));
      });
  INSPECT_STACK("i32.load16_u", wasmIns, executor);
}

// operands: [storeVal, baseAddr]; immes: [flags, offset].
void Interpreter::doI32StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int32_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int32_t storeVal) -> void {
      mem->store<int32_t>(ea, storeVal);
    });
  INSPECT_STACK("i32.store", wasmIns, executor);
}

void Interpreter::doI32StoreMem8(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int32_t>(wasmIns, executor, [](const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int32_t storeVal) -> void {
      mem->store<int8_t>(ea, (storeVal & 255u));
    });
  INSPECT_STACK("i32.store8", wasmIns, executor);
}

void Interpreter::doI32StoreMem16(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int32_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int32_t storeVal) -> void {
      mem->store<int8_t>(ea, (storeVal & 65535u));
    });
  INSPECT_STACK("i32.store16", wasmIns, executor);
}

void Interpreter::doI64LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(mem->load<int64_t>(ea));
      });
  INSPECT_STACK("i64.load", wasmIns, executor);
}

void Interpreter::doI64LoadMem8S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<int8_t>(ea)));
      });
  INSPECT_STACK("i64.load8_s", wasmIns, executor);
}

void Interpreter::doI64LoadMem8U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<uint8_t>(ea)));
      });
  INSPECT_STACK("i64.load8_u", wasmIns, executor);
}

void Interpreter::doI64LoadMem16S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<int16_t>(ea)));
      });
  INSPECT_STACK("i64.load16_s", wasmIns, executor);
}

void Interpreter::doI64LoadMem16U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<uint16_t>(ea)));
      });
  INSPECT_STACK("i64.load16_u", wasmIns, executor);
}

void Interpreter::doI64LoadMem32S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<int32_t>(ea)));
      });
  INSPECT_STACK("i64.load32_s", wasmIns, executor);
}

void Interpreter::doI64LoadMem32U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(
          static_cast<int64_t>(mem->load<uint32_t>(ea)));
      });
  INSPECT_STACK("i64.load32_u", wasmIns, executor);
}

void Interpreter::doI64StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int64_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int64_t storeVal) -> void {
      mem->store<int64_t>(ea, storeVal);
    });
  INSPECT_STACK("i64.store", wasmIns, executor);
}

void Interpreter::doI64StoreMem8(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int64_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int64_t storeVal) -> void {
      mem->store<int8_t>(ea, storeVal & 255u);
    });
  INSPECT_STACK("i64.store8", wasmIns, executor);
}

void Interpreter::doI64StoreMem16(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int64_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int64_t storeVal) -> void {
      mem->store<int16_t>(ea, storeVal & 65535u);
    });
  INSPECT_STACK("i64.store16", wasmIns, executor);
}

void Interpreter::doI64StoreMem32(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<int64_t>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const int64_t storeVal) -> void {
      mem->store<int32_t>(ea, storeVal & 4294836225u);
    });
  INSPECT_STACK("i64.store32", wasmIns, executor);
}

void Interpreter::doF32StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<float>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const float storeVal) -> void {
      mem->store<float>(ea, storeVal);
    });
  INSPECT_STACK("f32.store", wasmIns, executor);
}

void Interpreter::doF32LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(mem->load<float>(ea));
      });
  INSPECT_STACK("f32.load", wasmIns, executor);
}

void Interpreter::doF64StoreMem(shared_wasm_t &wasmIns, Executor *executor) {
  storeMemarg<double>(wasmIns, executor, [](
    const uint32_t ea,
    WasmMemoryInstance *const &mem,
    const double storeVal) -> void {
      mem->store<double>(ea, storeVal);
    });
  INSPECT_STACK("f64.store", wasmIns, executor);
}

void Interpreter::doF64LoadMem(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveMemarg(wasmIns, executor,
    [&wasmIns, &executor](
      const uint32_t ea,
      WasmMemoryInstance *const &mem) -> void {
        wasmIns->stack->valueStack->top() = executor->checkUpConstant(mem->load<double>(ea));
      });
  INSPECT_STACK("f64.load", wasmIns, executor);
}

// numerical comparison.
void Interpreter::doF32Floor(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(floor(x));
    });
  INSPECT_STACK("f32.floor", wasmIns, executor);
}

void Interpreter::doF32Trunc(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(trunc(x));
    });
  INSPECT_STACK("f32.trunc", wasmIns, executor);
}

void Interpreter::doF64Floor(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(floor(x));
    });
  INSPECT_STACK("f64.floor", wasmIns, executor);
}

void Interpreter::doF64Trunc(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(trunc(x));
    });
  INSPECT_STACK("f64.trunc", wasmIns, executor);
}

void Interpreter::doI32Popcnt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(Bits::countPopulation(x));
    });
  INSPECT_STACK("i32.popcnt", wasmIns, executor);
}

void Interpreter::doI64Popcnt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(Bits::countPopulation(x));
    });
  INSPECT_STACK("i64.popcnt", wasmIns, executor);
}

void Interpreter::doMemoryGrow(shared_wasm_t &wasmIns, Executor *executor) {
  // reserved byte for future usage.
  [[maybe_unused]] 
  const auto reseverdByte = executor->uint8UseImmesCache(
    [&executor](size_t *step, uint8_t *immediate) -> auto {
      *immediate = Decoder::readUint8(executor->forward_());
      executor->innerOffset += ((*step = 1) - 1);
    });
  retrieveSingleRTVal<uint32_t>(wasmIns, executor,
    [&executor, &wasmIns](const shared_ptr<Stack::ValueFrameStack> &valueStack, const uint32_t x) -> void {
      // -1 or "previous page size".
      wasmIns->stack->valueStack->top() = executor->checkUpConstant(
        wasmIns->module->memories[DEFAULT_ELEMENT_INDEX]->grow(x));
    });
  INSPECT_STACK("memory.grow", wasmIns, executor);
}

void Interpreter::doMemorySize(shared_wasm_t &wasmIns, Executor *executor) {
  // reserved byte for future usage.
  [[maybe_unused]] 
  const auto reseverdByte = executor->uint8UseImmesCache(
    [&executor](size_t *step, uint8_t *immediate) -> auto {
      *immediate = Decoder::readUint8(executor->forward_());
      executor->innerOffset += ((*step = 1) - 1);
    });
  // use the first one by default in MVP.
  wasmIns->stack->valueStack->push(executor->checkUpConstant(
    static_cast<uint32_t>(wasmIns->module->memories[DEFAULT_ELEMENT_INDEX]->getAvailablePage())));
  INSPECT_STACK("memory.size", wasmIns, executor);
}

void Interpreter::doF32CopySign(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<float>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const float x,
      const float y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(copysignf(x, y));
    });
  INSPECT_STACK("f32.copysign", wasmIns, executor);
}

void Interpreter::doF64CopySign(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<double>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const double x,
      const double y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(copysign(x, y));
    });
  INSPECT_STACK("f64.copysign", wasmIns, executor);
}

void Interpreter::doCallIndirect(shared_wasm_t &wasmIns, Executor *executor) {}

void Interpreter::doF32DemoteF64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(Utility::double64ToFloat32(x));
    });
  INSPECT_STACK("f32.demote_f64", wasmIns, executor);
}

void Interpreter::doF32NearestInt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(nearbyintf(x));
    });
  INSPECT_STACK("f32.nearest", wasmIns, executor);
}

void Interpreter::doF64PromoteF32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<double>(x));
    });
  INSPECT_STACK("f64.promote_f32", wasmIns, executor);
}

void Interpreter::doF64NearestInt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(nearbyintf(x));
    });
  INSPECT_STACK("f64.nearest", wasmIns, executor);
}

void Interpreter::doI32WrapI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(x & 0xffffffff));
    });
  INSPECT_STACK("i32.wrap_i64", wasmIns, executor);
}

void Interpreter::doF32SConvertI32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<float>(x));
    });
  INSPECT_STACK("f32.convert_i32_s", wasmIns, executor);
}

void Interpreter::doF32SConvertI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<float>(x));
    });
  INSPECT_STACK("f32.convert_i64_s", wasmIns, executor);
}

void Interpreter::doF32UConvertI32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(
        x < 0 ? static_cast<int64_t>(0) : static_cast<int64_t>(x));
    });
  INSPECT_STACK("f32.convert_i32_u", wasmIns, executor);
}

void Interpreter::doF32UConvertI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<uint64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const uint64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<float>(x));
    });
  INSPECT_STACK("f32.convert_i64_u", wasmIns, executor);
}

void Interpreter::doF64SConvertI32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<double>(x));
    });
  INSPECT_STACK("f64.convert_i32_s", wasmIns, executor);
}

void Interpreter::doF64SConvertI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<double>(x));
    });
  INSPECT_STACK("f64.convert_i64_s", wasmIns, executor);
}

void Interpreter::doF64UConvertI32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(
        x < 0 ? static_cast<double>(0) : static_cast<double>(x));
    });
  INSPECT_STACK("f64.convert_i32_u", wasmIns, executor);
}

void Interpreter::doF64UConvertI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<uint64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const uint64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<double>(x));
    });
  INSPECT_STACK("f64.convert_i64_u", wasmIns, executor);
}

void Interpreter::doI32TruncF32S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(x));
    });
  INSPECT_STACK("i32.trunc_f32_s", wasmIns, executor);
}

void Interpreter::doI32TruncF64S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(x));
    });
  INSPECT_STACK("i32.trunc_f64_s", wasmIns, executor);
}

void Interpreter::doI32TruncF32U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<uint32_t>(x));
    });
  INSPECT_STACK("i32.trunc_f32_u", wasmIns, executor);
}

void Interpreter::doI32TruncF64U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<uint32_t>(x));
    });
  INSPECT_STACK("i32.trunc_f64_u", wasmIns, executor);
}

void Interpreter::doI64TruncF32S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(x));
    });
  INSPECT_STACK("i64.trunc_f32_s", wasmIns, executor);
}

void Interpreter::doI64TruncF64S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(x));
    });
  INSPECT_STACK("i64.trunc_f64_s", wasmIns, executor);
}

void Interpreter::doI64ExtendI32S(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(x));
    });
  INSPECT_STACK("i64.extend_i32_s", wasmIns, executor);
}

void Interpreter::doI64TruncF32U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<uint64_t>(x));
    });
  INSPECT_STACK("i64.trunc_f32_u", wasmIns, executor);
}

void Interpreter::doI64TruncF64U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<uint64_t>(x));
    });
  INSPECT_STACK("i64.trunc_f64_u", wasmIns, executor);
}

void Interpreter::doI64ExtendI32U(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(
        x < 0 ? static_cast<int64_t>(0) : static_cast<int64_t>(x));
    });
  INSPECT_STACK("i64.extend_i32_u", wasmIns, executor);
}

void Interpreter::doF32ReinterpretI32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(*reinterpret_cast<float*>(const_cast<int32_t*>(&x)));
    });
  INSPECT_STACK("f32.reinterpret_i32", wasmIns, executor);
}

void Interpreter::doF64ReinterpretI64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(*reinterpret_cast<double*>(const_cast<int64_t*>(&x)));
    });
  INSPECT_STACK("f64.reinterpret_i64", wasmIns, executor);
}

void Interpreter::doI32ReinterpretF32(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(*reinterpret_cast<int32_t*>(const_cast<float*>(&x)));
    });
  INSPECT_STACK("i32.reinterpret_f32", wasmIns, executor);
}

void Interpreter::doI64ReinterpretF64(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(*reinterpret_cast<int64_t*>(const_cast<double*>(&x)));
    });
  INSPECT_STACK("i64.reinterpret_f64", wasmIns, executor);
}

void Interpreter::doNop(shared_wasm_t &wasmIns, Executor *executor) { 
  /* no effect */ 
  INSPECT_STACK("nop", wasmIns, executor); 
}

void Interpreter::doDrop(shared_wasm_t &wasmIns, Executor *executor) {
  // drop the top "ValueFrame" directly.
  wasmIns->stack->valueStack->pop();
  INSPECT_STACK("drop", wasmIns, executor);
}

void Interpreter::doF32Abs(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(abs(x));
    });
  INSPECT_STACK("f32.abs", wasmIns, executor);
}

void Interpreter::doF32Max(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<float>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const float x,
      const float y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(Utility::max<float>(x, y));
    });
  INSPECT_STACK("f32.max", wasmIns, executor);
}

void Interpreter::doF32Min(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<float>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const float x,
      const float y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(Utility::min<float>(x, y));
    });
  INSPECT_STACK("f32.min", wasmIns, executor);
}

void Interpreter::doF32Neg(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(-x);
    });
  INSPECT_STACK("f32.neg", wasmIns, executor);
}


void Interpreter::doF64Abs(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(abs(x));
    });
  INSPECT_STACK("f64.abs", wasmIns, executor);
}

void Interpreter::doF64Max(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<double>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const double x,
      const double y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(Utility::max<double>(x, y));
    });
  INSPECT_STACK("f64.max", wasmIns, executor);
}

void Interpreter::doF64Min(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<double>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const double x,
      const double y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(Utility::min<double>(x, y));
    });
  INSPECT_STACK("f64.min", wasmIns, executor);
}

void Interpreter::doF64Neg(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(-x);
    });
  INSPECT_STACK("f64.neg", wasmIns, executor);
}

void Interpreter::doI32Clz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(Bits::countLeadingZeros(x)));
    });
  INSPECT_STACK("i32.clz", wasmIns, executor);
}

void Interpreter::doI32Ctz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(Bits::countTrailingZeros(x)));
    });
  INSPECT_STACK("i32.ctz", wasmIns, executor);
}

void Interpreter::doI32Eqz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(x == 0 ? 1 : 0));
    });
  INSPECT_STACK("i32.eqz", wasmIns, executor);
}

void Interpreter::doI32Rotl(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant((x << (y & 0x1f)) | (x >> ((32 - y) & 0x1f)));
    });
  INSPECT_STACK("i32.rotl", wasmIns, executor);
}

void Interpreter::doI32Rotr(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant((x >> (y & 0x1f)) | (x << ((32 - y) & 0x1f)));
    });
  INSPECT_STACK("i32.rotr", wasmIns, executor);
}

void Interpreter::doI32Shl(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x << (y & 0x1f));
    });
  INSPECT_STACK("i32.shl", wasmIns, executor);
}

void Interpreter::doI64Clz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(Bits::countLeadingZeros(x)));
    });
  INSPECT_STACK("i64.clz", wasmIns, executor);
}

void Interpreter::doI64Ctz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(Bits::countTrailingZeros(x)));
    });
  INSPECT_STACK("i64.ctz", wasmIns, executor);
}

void Interpreter::doI64Eqz(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int64_t>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int64_t x) -> void {
      valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(x == 0 ? 1 : 0));
    });
  INSPECT_STACK("i64.eqz", wasmIns, executor);
}

void Interpreter::doI64Rotl(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant((x << (y & 0x3f)) | (x >> ((64 - y) & 0x3f)));
    });
  INSPECT_STACK("i64.rotl", wasmIns, executor);
}

void Interpreter::doI64Rotr(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant((x >> (y & 0x3f)) | (x << ((64 - y) & 0x3f)));
    });
  INSPECT_STACK("i64.rotr", wasmIns, executor);
}

void Interpreter::doI64Shl(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x << (y & 0x3f));
    });
  INSPECT_STACK("i64.shl", wasmIns, executor);
}

void Interpreter::doSelect(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<int32_t>(wasmIns, executor,
    [](const shared_ptr<Stack::ValueFrameStack> &valueStack, const int32_t x) -> void {
      if (x != 0) {
        /**
         * [operand][operand][select-key]
         *     ^--------^
         *      exchange
         */
        valueStack->at(valueStack->size() - 3) = valueStack->at(valueStack->size() - 2);
      }
      // remove the top 2 elements, including the flag ValueFrame.
      valueStack->popN(2);
    });
  INSPECT_STACK("select", wasmIns, executor);
}

void Interpreter::doF32Ceil(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(ceilf(x));
    });
  INSPECT_STACK("f32.ceil", wasmIns, executor);
}

void Interpreter::doF32Sqrt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<float>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const float x) -> void {
      valueStack->top() = executor->checkUpConstant(
        static_cast<float>(sqrtf(x)));
    });
  INSPECT_STACK("f32.sqrt", wasmIns, executor);
}

void Interpreter::doF64Ceil(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(
        static_cast<double>(ceill(x)));
    });
  INSPECT_STACK("f64.ceil", wasmIns, executor);
}

void Interpreter::doF64Sqrt(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveSingleRTVal<double>(wasmIns, executor,
    [&executor](const shared_ptr<Stack::ValueFrameStack> &valueStack, const double x) -> void {
      valueStack->top() = executor->checkUpConstant(sqrt(x));
    });
  INSPECT_STACK("f64.sqrt", wasmIns, executor);
}

void Interpreter::doI32DivS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else if (y == -1 && x == numeric_limits<int32_t>::min()) {
        // INT32_T_MIN = -INT32_T_MAX - 1.
        Printer::instance().error(Errors::RT_DIV_UNREPRESENTABLE);
      } else {
        valueStack->top() = executor->checkUpConstant(x / y);
      }
    });
  INSPECT_STACK("i32.div_s", wasmIns, executor);
}

void Interpreter::doI32DivU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint32_t x,
      const uint32_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else {
        valueStack->top() = executor->checkUpConstant(x / y);
      }
    });
  INSPECT_STACK("i32.div_u", wasmIns, executor);
}

void Interpreter::doI32RemS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else if (y == -1) {
        valueStack->top() = executor->checkUpConstant(static_cast<int32_t>(0));
      } else {
        valueStack->top() = executor->checkUpConstant(x % y);
      }
    });
  INSPECT_STACK("i32.rem_s", wasmIns, executor);
}

void Interpreter::doI32RemU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint32_t x,
      const uint32_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else {
        valueStack->top() = executor->checkUpConstant(x % y);
      }
    });
  INSPECT_STACK("i32.rem_u", wasmIns, executor);
}

void Interpreter::doI32ShrS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int32_t x,
      const int32_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x >> (y & 0x1f));
    });
  INSPECT_STACK("i32.shr_s", wasmIns, executor);
}

void Interpreter::doI32ShrU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint32_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint32_t x,
      const uint32_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x >> (y & 0x1f));
    });
  INSPECT_STACK("i32.shr_u", wasmIns, executor);
}

void Interpreter::doI64DivS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else if (y == -1 && x == numeric_limits<int64_t>::min()) {
        // INT64_T_MIN = -INT64_T_MAX - 1.
        Printer::instance().error(Errors::RT_DIV_UNREPRESENTABLE);
      } else {
        valueStack->top() = executor->checkUpConstant(x / y);
      }
    });
  INSPECT_STACK("i64.div_s", wasmIns, executor);
}

void Interpreter::doI64DivU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint64_t x,
      const uint64_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else {
        valueStack->top() = executor->checkUpConstant(x / y);
      }
    });
  INSPECT_STACK("i64.div_u", wasmIns, executor);
}

void Interpreter::doI64RemS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else if (y == -1) {
        valueStack->top() = executor->checkUpConstant(static_cast<int64_t>(0));
      } else {
        valueStack->top() = executor->checkUpConstant(y == -1 ? static_cast<int64_t>(0) : x % y);
      }
    });
  INSPECT_STACK("i64.rem_s", wasmIns, executor);
}

void Interpreter::doI64RemU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint64_t x,
      const uint64_t y) -> void {
      valueStack->pop();
      if (y == 0) {
        Printer::instance().error(Errors::RT_DIV_BY_ZERO);
      } else {
        valueStack->top() = executor->checkUpConstant(x % y);
      }
    });
  INSPECT_STACK("i64.rem_u", wasmIns, executor);
}

void Interpreter::doI64ShrS(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<int64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const int64_t x,
      const int64_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x >> (y & 0x3f));
    });
  INSPECT_STACK("i64.shr_s", wasmIns, executor);
}

void Interpreter::doI64ShrU(shared_wasm_t &wasmIns, Executor *executor) {
  retrieveDoubleRTVals<uint64_t>(wasmIns, executor,
    [&executor](
      const shared_ptr<Stack::ValueFrameStack> &valueStack,
      const uint64_t x,
      const uint64_t y) -> void {
      valueStack->pop();
      valueStack->top() = executor->checkUpConstant(x >> (y & 0x3f));
    });
  INSPECT_STACK("i64.shr_u", wasmIns, executor);
}
