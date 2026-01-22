// Copyright 2021 YHSPY. All rights reserved.
#include <array>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cstdlib>
#include <cmath>
#include <cfenv>
#include <limits>
#include "lib/include/interpreter.hh"
#include "lib/include/structs.hh"
#include "lib/include/executor.hh"
#include "lib/include/exception.hh"
#include "lib/include/opcodes.hh"
#include "lib/include/util.hh"

#define ITERATE_LOAD_MEMOP(V) \
  V(I32LoadMem, rt_i32_t, Runtime::rt_i32_t) \
  V(I64LoadMem, rt_i64_t, Runtime::rt_i64_t) \
  V(F32LoadMem, rt_f32_t, Runtime::rt_f32_t) \
  V(F64LoadMem, rt_f64_t, Runtime::rt_f64_t) \
  V(I32LoadMem8S, rt_i32_t, int8_t) \
  V(I32LoadMem8U, rt_i32_t, uint8_t) \
  V(I32LoadMem16S, rt_i32_t, int16_t) \
  V(I32LoadMem16U, rt_i32_t, uint16_t) \
  V(I64LoadMem8S, rt_i64_t, int8_t) \
  V(I64LoadMem8U, rt_i64_t, uint8_t) \
  V(I64LoadMem16S, rt_i64_t, int16_t) \
  V(I64LoadMem16U, rt_i64_t, uint16_t) \
  V(I64LoadMem32S, rt_i64_t, int32_t) \
  V(I64LoadMem32U, rt_i64_t, uint32_t)

#define ITERATE_STORE_MEMOP(V) \
  V(I32StoreMem, rt_i32_t, Runtime::rt_i32_t) \
  V(I64StoreMem, rt_i64_t, Runtime::rt_i64_t) \
  V(F32StoreMem, rt_f32_t, Runtime::rt_f32_t) \
  V(F64StoreMem, rt_f64_t, Runtime::rt_f64_t) \
  V(I32StoreMem8, rt_i32_t, uint8_t) \
  V(I32StoreMem16, rt_i32_t, uint16_t) \
  V(I64StoreMem8, rt_i64_t, uint8_t) \
  V(I64StoreMem16, rt_i64_t, uint16_t) \
  V(I64StoreMem32, rt_i64_t, uint32_t)

#define ITERATE_TRUNCOP(V) \
  V(I32TruncF32S, rt_f32_t, rt_i32_t, int32_t) \
  V(I32TruncF32U, rt_f32_t, rt_i32_t, uint32_t) \
  V(I32TruncF64S, rt_f64_t, rt_i32_t, int32_t) \
  V(I32TruncF64U, rt_f64_t, rt_i32_t, uint32_t) \
  V(I64TruncF32S, rt_f32_t, rt_i64_t, int64_t) \
  V(I64TruncF32U, rt_f32_t, rt_i64_t, uint64_t) \
  V(I64TruncF64S, rt_f64_t, rt_i64_t, int64_t) \
  V(I64TruncF64U, rt_f64_t, rt_i64_t, uint64_t)

#define ITERATE_CONVERTOP(V) \
  V(I64ExtendI32S, rt_i32_t, rt_i64_t, SIGNED) \
  V(I64ExtendI32U, rt_i32_t, rt_i64_t, UNSIGNED) \
  V(F32SConvertI32, rt_i32_t, rt_f32_t, SIGNED) \
  V(F32UConvertI32, rt_i32_t, rt_f32_t, UNSIGNED) \
  V(F32SConvertI64, rt_i64_t, rt_f32_t, SIGNED) \
  V(F32UConvertI64, rt_i64_t, rt_f32_t, UNSIGNED) \
  V(F64SConvertI32, rt_i32_t, rt_f64_t, SIGNED) \
  V(F64UConvertI32, rt_i32_t, rt_f64_t, UNSIGNED) \
  V(F64SConvertI64, rt_i64_t, rt_f64_t, SIGNED) \
  V(F64UConvertI64, rt_i64_t, rt_f64_t, UNSIGNED)

#define ITERATE_REINTERPRETOP(V) \
  V(I32ReinterpretF32, rt_f32_t, rt_i32_t) \
  V(I64ReinterpretF64, rt_f64_t, rt_i64_t) \
  V(F32ReinterpretI32, rt_i32_t, rt_f32_t) \
  V(F64ReinterpretI64, rt_i64_t, rt_f64_t) \

#define ITERATE_SIMPLE_BINOP(V) \
  V(I32Mul, rt_i32_t, rt_i32_t, rt_i32_t, *) \
  V(I32Add, rt_i32_t, rt_i32_t, rt_i32_t, +) \
  V(I32Sub, rt_i32_t, rt_i32_t, rt_i32_t, -) \
  V(I32And, rt_i32_t, rt_i32_t, rt_i32_t, &) \
  V(I32Or, rt_i32_t, rt_i32_t, rt_i32_t, |) \
  V(I32Xor, rt_i32_t, rt_i32_t, rt_i32_t, ^) \
  V(I32Eq, rt_i32_t, rt_i32_t, rt_i32_t, ==) \
  V(I32Ne, rt_i32_t, rt_i32_t, rt_i32_t, !=) \
  V(I32LtU, rt_i32_t, rt_i32_t, imme_u32_t, <) \
  V(I32LeU, rt_i32_t, rt_i32_t, imme_u32_t, <=) \
  V(I32GtU, rt_i32_t, rt_i32_t, imme_u32_t, >) \
  V(I32GeU, rt_i32_t, rt_i32_t, imme_u32_t, >=) \
  V(I32LtS, rt_i32_t, rt_i32_t, rt_i32_t, <) \
  V(I32LeS, rt_i32_t, rt_i32_t, rt_i32_t, <=) \
  V(I32GtS, rt_i32_t, rt_i32_t, rt_i32_t, >) \
  V(I32GeS, rt_i32_t, rt_i32_t, rt_i32_t, >=) \
  V(I64Mul, rt_i64_t, rt_i64_t, rt_i64_t, *) \
  V(I64Add, rt_i64_t, rt_i64_t, rt_i64_t, +) \
  V(I64Sub, rt_i64_t, rt_i64_t, rt_i64_t, -) \
  V(I64And, rt_i64_t, rt_i64_t, rt_i64_t, &) \
  V(I64Or, rt_i64_t, rt_i64_t, rt_i64_t, |) \
  V(I64Xor, rt_i64_t, rt_i64_t, rt_i64_t, ^) \
  V(I64Eq, rt_i64_t, rt_i32_t, rt_i64_t, ==) \
  V(I64Ne, rt_i64_t, rt_i32_t, rt_i64_t, !=) \
  V(I64LtU, rt_i64_t, rt_i32_t, imme_u64_t, <) \
  V(I64LeU, rt_i64_t, rt_i32_t, imme_u64_t, <=) \
  V(I64GtU, rt_i64_t, rt_i32_t, imme_u64_t, >) \
  V(I64GeU, rt_i64_t, rt_i32_t, imme_u64_t, >=) \
  V(I64LtS, rt_i64_t, rt_i32_t, rt_i64_t, <) \
  V(I64LeS, rt_i64_t, rt_i32_t, rt_i64_t, <=) \
  V(I64GtS, rt_i64_t, rt_i32_t, rt_i64_t, >) \
  V(I64GeS, rt_i64_t, rt_i32_t, rt_i64_t, >=) \
  V(F32Mul, rt_f32_t, rt_f32_t, rt_f32_t, *) \
  V(F32Add, rt_f32_t, rt_f32_t, rt_f32_t, +) \
  V(F32Sub, rt_f32_t, rt_f32_t, rt_f32_t, -) \
  V(F32Div, rt_f32_t, rt_f32_t, rt_f32_t, /) \
  V(F32Eq, rt_f32_t, rt_i32_t, rt_f32_t, ==) \
  V(F32Ne, rt_f32_t, rt_i32_t, rt_f32_t, !=) \
  V(F32Lt, rt_f32_t, rt_i32_t, rt_f32_t, <) \
  V(F32Le, rt_f32_t, rt_i32_t, rt_f32_t, <=) \
  V(F32Gt, rt_f32_t, rt_i32_t, rt_f32_t, >) \
  V(F32Ge, rt_f32_t, rt_i32_t, rt_f32_t, >=) \
  V(F64Mul, rt_f64_t, rt_f64_t, rt_f64_t, *) \
  V(F64Add, rt_f64_t, rt_f64_t, rt_f64_t, +) \
  V(F64Sub, rt_f64_t, rt_f64_t, rt_f64_t, -) \
  V(F64Div, rt_f64_t, rt_f64_t, rt_f64_t, /) \
  V(F64Eq, rt_f64_t, rt_i32_t, rt_f64_t, ==) \
  V(F64Ne, rt_f64_t, rt_i32_t, rt_f64_t, !=) \
  V(F64Lt, rt_f64_t, rt_i32_t, rt_f64_t, <) \
  V(F64Le, rt_f64_t, rt_i32_t, rt_f64_t, <=) \
  V(F64Gt, rt_f64_t, rt_i32_t, rt_f64_t, >) \
  V(F64Ge, rt_f64_t, rt_i32_t, rt_f64_t, >=)

#define REF_OPCODE_HANDLER_PTR_VALID(NAME) \
  Interpreter::do##NAME,
#define REF_OPCODE_HANDLER_PTR_INVALID(NAME) \
  nullptr,
#define REF_OPCODE_HANDLER_PTR(NAME, OP, VALIDITY) \
  REF_OPCODE_HANDLER_PTR_##VALIDITY(NAME)
#define CONCAT_PREFIX(X) Runtime:: X
#define MAKE_UNSIGNED() std::make_unsigned_t
#define MAKE_SIGNED() std::make_signed_t
#define DECLARE_BASIC_BINOP_METHOD(NAME, VAL_TYPE, RET_TYPE, OP_CAST_TYPE, OP) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    executor.opHandlerFTRO<CONCAT_PREFIX(VAL_TYPE), CONCAT_PREFIX(RET_TYPE)>([](auto x, auto y) { \
      return static_cast<CONCAT_PREFIX(OP_CAST_TYPE)>(x) OP static_cast<CONCAT_PREFIX(OP_CAST_TYPE)>(y); \
    }); \
  }
#define DECLARE_MEM_LOAD_OP_METHOD(NAME, T, C) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    const auto& defaultMem = executor.getEngineData()->rtMems.front(); \
    const auto [flags, offset] = executor.parseMemImmeInfo(); \
    const auto ea = executor.retStackValOfRTType<Runtime::rt_i32_t>(false) + offset; \
    const auto n = sizeof(CONCAT_PREFIX(T)); \
    if (ea + n / 8 <= defaultMem.size) { \
      executor.refFrameFromStack<Runtime::RTValueFrame>().value = \
        static_cast<CONCAT_PREFIX(T)>(*reinterpret_cast<C*>(defaultMem.ptr + ea)); \
    } else { \
      Exception::terminate(Exception::ErrorType::MEM_ACCESS_OOB); \
    } \
  }
#define DECLARE_MEM_STORE_OP_METHOD(NAME, T, C) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    const auto& defaultMem = executor.getEngineData()->rtMems.front(); \
    const auto [flags, offset] = executor.parseMemImmeInfo(); \
    const auto c = executor.retStackValOfRTType<CONCAT_PREFIX(T)>(); \
    const auto ea = executor.retStackValOfRTType<Runtime::rt_i32_t>() + offset; \
    const auto n = sizeof(CONCAT_PREFIX(T)); \
    if (ea + n / 8 <= defaultMem.size) { \
      *reinterpret_cast<C*>(defaultMem.ptr + ea) = static_cast<C>(c); \
    } else { \
      Exception::terminate(Exception::ErrorType::MEM_ACCESS_OOB); \
    } \
  }
#define DECLARE_TRUNC_OP_METHOD(NAME, PARAM_TYPE, RET_TYPE, CAST) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    executor.opHandlerFORO<CONCAT_PREFIX(PARAM_TYPE), CONCAT_PREFIX(RET_TYPE)>([](auto v) { \
      v = std::trunc(v); \
      if (!std::isnan(v) && \
          !std::isinf(v) && \
          Util::floatInRange<CAST>(v)) { \
          return static_cast<CAST>(v); \
        } else { \
          Exception::terminate(Exception::ErrorType::FLOAT_UNREPRESENTABLE); \
        } \
    }); \
  }
#define DECLARE_CONVERT_OP_METHOD(NAME, PARAM_TYPE, RET_TYPE, SIGNESS_METHOD) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    executor.opHandlerFORO<CONCAT_PREFIX(PARAM_TYPE), CONCAT_PREFIX(RET_TYPE)>([](auto v) { \
      return static_cast<MAKE_##SIGNESS_METHOD()<decltype(v)>>(v); \
    }); \
  }
#define DECLARE_REINTERPRET_OP_METHOD(NAME, PARAM_TYPE, RET_TYPE) \
  void Interpreter::do##NAME(Executor& executor, op_handler_info_t _) { \
    executor.opHandlerFORO<CONCAT_PREFIX(PARAM_TYPE), CONCAT_PREFIX(RET_TYPE)>([](auto v) { \
      return *reinterpret_cast<CONCAT_PREFIX(RET_TYPE)*>(static_cast<CONCAT_PREFIX(PARAM_TYPE)*>(&v)); \
    }); \
  }

namespace TWVM {

std::array<Interpreter::op_handler_proto_t, sizeof(uint8_t) * 1 << 8> Interpreter::opTokenHandlers = {
  ITERATE_ALL_OPCODE(REF_OPCODE_HANDLER_PTR)
};

ITERATE_SIMPLE_BINOP(DECLARE_BASIC_BINOP_METHOD)
ITERATE_LOAD_MEMOP(DECLARE_MEM_LOAD_OP_METHOD)
ITERATE_STORE_MEMOP(DECLARE_MEM_STORE_OP_METHOD)
ITERATE_TRUNCOP(DECLARE_TRUNC_OP_METHOD)
ITERATE_CONVERTOP(DECLARE_CONVERT_OP_METHOD)
ITERATE_REINTERPRETOP(DECLARE_REINTERPRET_OP_METHOD)

void Interpreter::doUnreachable(Executor& executor, op_handler_info_t _) {
  Exception::terminate(Exception::ErrorType::UNREACHABLE);
}
void Interpreter::doNop(Executor& executor, op_handler_info_t _) {}
void Interpreter::doBlock(Executor& executor, op_handler_info_t _) {
  const auto returnArityTypes = executor.collectArities();
  const auto cont = executor.lookupLabelContFromPC();
  if (cont.size() > 0) {
    executor.pushToStack(Runtime::RTLabelFrame(cont.back(), returnArityTypes));
  } else  {
    Exception::terminate(Exception::ErrorType::ILLFORMED_STRUCTURE);
  }
}
void Interpreter::doLoop(Executor& executor, op_handler_info_t _) {
  auto* cont = executor.getPC() - 1;
  const auto returnArityTypes = executor.collectArities();
  executor.pushToStack(Runtime::RTLabelFrame(cont, returnArityTypes));
}
void Interpreter::doIf(Executor& executor, op_handler_info_t _) {
  const auto returnArityTypes = executor.collectArities();
  const auto conts = executor.lookupLabelContFromPC();
  if (conts.size() > 1) {
    const auto v = executor.retStackValOfRTType<Runtime::rt_i32_t>();
    executor.pushToStack(Runtime::RTLabelFrame(conts.back(), returnArityTypes));
    if (v == 0) {
      executor.setPC(conts.front());
    }
  } else  {
    Exception::terminate(Exception::ErrorType::ILLFORMED_STRUCTURE);
  }
}
void Interpreter::doElse(Executor& executor, op_handler_info_t _) {
  doBr(executor, 0);
}
void Interpreter::doEnd(Executor& executor, op_handler_info_t _) {
  doBr(executor, 0);  // No forwarding PC.
}
void Interpreter::doBr(Executor& executor, op_handler_info_t passedDepth) {
  const auto depth = passedDepth.has_value() ? *passedDepth : executor.decodeVaruintFromPC<Runtime::relative_depth_t>();
  const auto labelsCount = executor.getLabelAboveActivFrameCount();
  if (labelsCount > depth) {
    // Consume Label frames.
    auto* cont = executor.retFromFrameWithCont<Runtime::RTLabelFrame>(depth);
    if (static_cast<OpCodes>(*(executor.getPC() - 1)) != OpCodes::End) {
      executor.setPC(cont);
    }
  } else if (labelsCount == depth) {
    // Consuem Activ frame.
    doReturn(executor, depth);
  } else {
    Exception::terminate(Exception::ErrorType::ILLEGAL_BREAK_LVL);
  }
}
void Interpreter::doBrIf(Executor& executor, op_handler_info_t _) {
  const auto v = executor.retStackValOfRTType<Runtime::rt_i32_t>();
  const auto depth = executor.decodeVaruintFromPC<Runtime::relative_depth_t>();
  if (v != 0) {
    doBr(executor, depth);
  }
}
void Interpreter::doBrTable(Executor& executor, op_handler_info_t _) {
  const auto entries = executor.parseBrTableInfo();
  const auto v = executor.retStackValOfRTType<Runtime::rt_i32_t>();
  if (v < entries.size()) {
    doBr(executor, entries.at(v));
  } else {
    doBr(executor, entries.back());
  }
}
void Interpreter::doReturn(Executor& executor, op_handler_info_t labelDepth) {
  const auto activIdx = executor.getTopFrameIdx(Runtime::STVariantIndex::ACTIVATION);
  if (activIdx.has_value() && *activIdx == 0) {
    executor.stopEngine();
  } else {
    const auto depth = labelDepth.value_or(executor.getLabelAboveActivFrameCount());
    executor.setPC(
      executor.retFromFrameWithCont<Runtime::RTActivFrame>(depth));
  }
}
void Interpreter::doCall(Executor& executor, op_handler_info_t passedFuncIdx) {
  const auto idx = passedFuncIdx.has_value() ? *passedFuncIdx : executor.decodeVaruintFromPC<Runtime::index_t>();
  const auto& descriptor = executor.getEngineData()->rtFuncDescriptor.at(idx);
  auto paramCount = descriptor.funcType->first.size();
  auto rtLocals = descriptor.localsDefault;  // Copied.
  // Set up func parameters.
  if (paramCount > 0) {
    for (auto i = 0; i < paramCount; ++i) {
      const auto& vf = executor.refFrameFromStack<Runtime::RTValueFrame>();
      if (vf.value.index() == rtLocals.at(i).index()) {
        rtLocals.at(i) = vf.value;
        executor.popFromStack();
      } else {
        Exception::terminate(Exception::ErrorType::MISSING_FUNC_PARAMS);
      }
    }
  }
  // Construct frame (locals + artiy).
  executor.pushToStack(
    Runtime::RTActivFrame(
      rtLocals,
      executor.getPC(),
      &descriptor.funcType->second));
  // Redirection.
  executor.setPC(descriptor.codeEntry);
}
void Interpreter::doCallIndirect(Executor& executor, op_handler_info_t _) {
  const auto& engineData = executor.getEngineData();
  const auto& defaultTable = engineData->rtTables.front();  // Restricted to only 1 table in MVP.
  const auto sigIdx = executor.decodeVaruintFromPC<Runtime::index_t>();
  [[maybe_unused]] const auto tblIdx = executor.decodeByteFromPC();  // reserved.
  const auto& funcTypesRef = engineData->module->funcTypes;
  if (funcTypesRef.size() > sigIdx) {
    const auto& expectedType = funcTypesRef.at(sigIdx);
    const auto funcIdx = executor.retStackValOfRTType<Runtime::rt_i32_t>();
    if (defaultTable.size() > funcIdx) {
      executor.validateTypeWithFuncIdx(expectedType, funcIdx);  // May throw.
      doCall(executor, funcIdx);
    } else {
      Exception::terminate(Exception::ErrorType::NO_AVAILABLE_TABLES_EXIST);
    }
  } else {
    Exception::terminate(Exception::ErrorType::FUNC_TYPE_ACCESS_OOB);
  }
}
void Interpreter::doDrop(Executor& executor, op_handler_info_t _) {
  executor.refFrameFromStack<Runtime::RTValueFrame>();
  executor.popFromStack();
}
void Interpreter::doSelect(Executor& executor, op_handler_info_t _) {
  const auto v = executor.retStackValOfRTType<Runtime::rt_i32_t>();
  const auto& vy = executor.refFrameFromStack<Runtime::RTValueFrame>();  // Top.
  const auto& vx = executor.refFrameFromStack<Runtime::RTValueFrame>(1);
  if (vy.value.index() == vx.value.index()) {
    executor.eraseFrameFromStack(v == 0 ? 1 : 0);
  } else {
    Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
  }
}
void Interpreter::doLocalGet(Executor& executor, op_handler_info_t _) {
  const auto idx = executor.decodeVaruintFromPC<Runtime::index_t>();
  const auto& frameOffset = executor.refTrackedTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
  const auto& locals = std::get<Runtime::RTActivFrame>(*frameOffset.ptr).locals;
  if (locals.size() >= idx + 1) {
    executor.pushToStack(locals.at(idx));
  } else {
    Exception::terminate(Exception::ErrorType::ILLEGAL_LOCAL_IDX);
  }
}
void Interpreter::doLocalSet(Executor& executor, op_handler_info_t fromTee) {
  const auto localIdx = executor.decodeVaruintFromPC<Runtime::index_t>();
  const auto& activFrameOffset = executor.refTrackedTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
  const auto& valueFrame = executor.refFrameFromStack<Runtime::RTValueFrame>();
  auto& locals = std::get<Runtime::RTActivFrame>(*activFrameOffset.ptr).locals;
  if (locals.at(localIdx).index() == valueFrame.value.index()) {
    locals.at(localIdx) = valueFrame.value;
  } else {
    Exception::terminate(Exception::ErrorType::STACK_VAL_TYPE_MISMATCH);
  }
  if (!fromTee.has_value()) {
    executor.popFromStack();
  }
}
void Interpreter::doLocalTee(Executor& executor, op_handler_info_t _) {
  doLocalSet(executor, OPTIONAL_SYM_BOOL_TRUE);
}
void Interpreter::doGlobalGet(Executor& executor, op_handler_info_t _) {
  const auto idx = executor.decodeVaruintFromPC<Runtime::index_t>();
  auto& rtGlobals = executor.getEngineData()->rtGlobals;
  if (rtGlobals.size() > idx) {
    executor.pushToStack(Runtime::RTValueFrame(rtGlobals.at(idx)));
  } else {
    Exception::terminate(Exception::ErrorType::GLOBAL_ACCESS_OOB);
  }
}
void Interpreter::doGlobalSet(Executor& executor, op_handler_info_t _) {
  const auto idx = executor.decodeVaruintFromPC<Runtime::index_t>();
  const auto& engineData = executor.getEngineData();
  auto& rtGlobals = engineData->rtGlobals;
  if (rtGlobals.size() > idx) {
    const auto mutability = engineData->module->globals.at(idx).globalType.mutability;
    const auto& v = executor.refFrameFromStack<Runtime::RTValueFrame>();
    if (mutability && v.value.index() == rtGlobals.at(idx).index()) {
      rtGlobals.at(idx) = v.value;
      executor.popFromStack();
    } else {
      Exception::terminate(Exception::ErrorType::IMMUTABLE_GLOBAL_MUTATION);
    }
  } else {
    Exception::terminate(Exception::ErrorType::GLOBAL_ACCESS_OOB);
  }
}
void Interpreter::doI32Const(Executor& executor, op_handler_info_t _) {
  executor.pushToStack(
    Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i32_t, Runtime::runtime_value_t>()));
}
void Interpreter::doI64Const(Executor& executor, op_handler_info_t _) {
  executor.pushToStack(
    Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i64_t, Runtime::runtime_value_t>()));
}
void Interpreter::doF32Const(Executor& executor, op_handler_info_t _) {
  executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<Runtime::rt_f32_t>()));
}
void Interpreter::doF64Const(Executor& executor, op_handler_info_t _) {
  executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<Runtime::rt_f64_t>()));
}
void Interpreter::doMemorySize(Executor& executor, op_handler_info_t _) {
  const auto& rtMems = executor.getEngineData()->rtMems;
  [[maybe_unused]] const auto reserved = executor.decodeByteFromPC();
  if (rtMems.size() > 0) {
    const auto& defaultMem = rtMems.front();
    executor.pushToStack(
      Runtime::RTValueFrame(
        static_cast<Runtime::rt_i32_t>(defaultMem.size)));
  } else {
    Exception::terminate(Exception::ErrorType::NO_AVAILABLE_MEM);
  }
}
void Interpreter::doMemoryGrow(Executor& executor, op_handler_info_t _) {
  const auto& rtMems = executor.getEngineData()->rtMems;
  [[maybe_unused]] const auto reserved = executor.decodeByteFromPC();
  if (rtMems.size() > 0) {
    const auto& defaultMem = rtMems.front();
    const auto sz = defaultMem.size / WASM_PAGE_SIZE_IN_BYTE;
    const auto n = executor.retStackValOfRTType<Runtime::rt_i32_t>();
    const auto size = n + sz;
    executor.pushToStack(
      Runtime::RTValueFrame(static_cast<Runtime::rt_i32_t>(executor.resizeMem(size))));
  } else {
    Exception::terminate(Exception::ErrorType::NO_AVAILABLE_MEM);
  }
}
void Interpreter::doI32Eqz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto v) {
    return v == 0;
  });
}
void Interpreter::doI64Eqz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i64_t, Runtime::rt_i32_t>([](auto v) {
    return v == 0;
  });
}
void Interpreter::doI32Clz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto v) {
    return Util::countLeadingZeros(v);
  });
}
void Interpreter::doI32Ctz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto v) {
    return Util::countTrailingZeros(v);
  });
}
void Interpreter::doI32Popcnt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto v) {
    return Util::countPopulation(v);
  });
}
void Interpreter::doI32DivS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    if (sy != 0) {
      if (!(sy == -1 && sx == std::numeric_limits<int32_t>::min())) {
        return sx / sy;
      } else {
        Exception::terminate(Exception::ErrorType::VAL_NOT_REPRESENTABLE);
      }
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI32DivU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    if (uy != 0) {
      return ux / uy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI32RemS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    if (sy != 0) {
      return sx % sy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI32RemU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    if (uy != 0) {
      return ux % uy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI32Shl(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    return x << (y & 0x1f);
  });
}
void Interpreter::doI32ShrS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    return sx >> (sy & 0x1f);
  });
}
void Interpreter::doI32ShrU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    return ux >> (uy & 0x1f);
  });
}
void Interpreter::doI32Rotl(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    return (ux << (y & 0x1f)) | (ux >> (32 - (y & 0x1f)));
  });
}
void Interpreter::doI32Rotr(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i32_t, Runtime::rt_i32_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    return (ux >> (y & 0x1f)) | (ux << ((32 - y) & 0x1f));
  });
}
void Interpreter::doI64Clz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto v) {
    return Util::countLeadingZeros(v);
  });
}
void Interpreter::doI64Ctz(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto v) {
    return Util::countTrailingZeros(v);
  });
}
void Interpreter::doI64Popcnt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto v) {
    return Util::countPopulation(v);
  });
}
void Interpreter::doI64DivS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    if (sy != 0) {
      if (!(sy == -1 && sx == std::numeric_limits<int64_t>::min())) {
        return sx / sy;
      } else {
        Exception::terminate(Exception::ErrorType::VAL_NOT_REPRESENTABLE);
      }
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI64DivU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    if (uy != 0) {
      return ux / uy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI64RemS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    if (sy != 0) {
      return sx % sy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI64RemU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    if (uy != 0) {
      return ux % uy;
    } else {
      Exception::terminate(Exception::ErrorType::DIVISION_BY_ZERO);
    }
  });
}
void Interpreter::doI64Shl(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    return x << (y & 0x3f);
  });
}
void Interpreter::doI64ShrS(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto sx = static_cast<std::make_signed_t<decltype(x)>>(x);
    const auto sy = static_cast<std::make_signed_t<decltype(y)>>(y);
    return sx >> (sy & 0x3f);
  });
}
void Interpreter::doI64ShrU(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    const auto uy = static_cast<std::make_unsigned_t<decltype(y)>>(y);
    return ux >> (uy & 0x3f);
  });
}
void Interpreter::doI64Rotl(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    return (ux << (y & 0x3f)) | (ux >> (64 - (y & 0x3f)));
  });
}
void Interpreter::doI64Rotr(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_i64_t, Runtime::rt_i64_t>([](auto x, auto y) {
    const auto ux = static_cast<std::make_unsigned_t<decltype(x)>>(x);
    return (ux >> (y & 0x3f)) | (ux << ((64 - y) & 0x3f));
  });
}
void Interpreter::doF32Abs(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return std::abs(v);
  });
}
void Interpreter::doF32Neg(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return -v;
  });
}
void Interpreter::doF32Ceil(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return std::ceil(v);
  });
}
void Interpreter::doF32Floor(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return std::floor(v);
  });
}
void Interpreter::doF32Trunc(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return std::trunc(v);
  });
}
void Interpreter::doF32NearestInt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    std::fesetround(FE_TONEAREST);
    return std::nearbyint(v);
  });
}
void Interpreter::doF32Sqrt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto v) {
    return std::sqrt(v);
  });
}
void Interpreter::doF32Min(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto x, auto y) {
    return std::min(x, y);
  });
}
void Interpreter::doF32Max(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto x, auto y) {
    return std::max(x, y);
  });
}
void Interpreter::doF32CopySign(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f32_t, Runtime::rt_f32_t>([](auto x, auto y) {
    return std::copysign(x, y);
  });
}
void Interpreter::doF64Abs(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return std::abs(v);
  });
}
void Interpreter::doF64Neg(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return -v;
  });
}
void Interpreter::doF64Ceil(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return std::ceil(v);
  });
}
void Interpreter::doF64Floor(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return std::floor(v);
  });
}
void Interpreter::doF64Trunc(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return std::trunc(v);
  });
}
void Interpreter::doF64NearestInt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    std::fesetround(FE_TONEAREST);
    return std::nearbyint(v);
  });
}
void Interpreter::doF64Sqrt(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto v) {
    return std::sqrt(v);
  });
}
void Interpreter::doF64Min(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto x, auto y) {
    return std::min(x, y);
  });
}
void Interpreter::doF64Max(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto x, auto y) {
    return std::max(x, y);
  });
}
void Interpreter::doF64CopySign(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFTRO<Runtime::rt_f64_t, Runtime::rt_f64_t>([](auto x, auto y) {
    return std::copysign(x, y);
  });
}
void Interpreter::doI32WrapI64(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_i64_t, Runtime::rt_i32_t>([](auto v) {
    return v & 0xffffffff;
  });
}
void Interpreter::doF32DemoteF64(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f64_t, Runtime::rt_f32_t>([](auto v) {
    return v;
  });
}
void Interpreter::doF64PromoteF32(Executor& executor, op_handler_info_t _) {
  executor.opHandlerFORO<Runtime::rt_f32_t, Runtime::rt_f64_t>([](auto v) {
    return v;
  });
}

// Direct-threaded interpreter using computed goto (GCC/Clang extension)
// This provides significantly better performance than function pointer dispatch
void Interpreter::executeDirectThreaded(Executor& executor) {
#if defined(__GNUC__) || defined(__clang__)
  // Check if we support computed goto
  #define USE_COMPUTED_GOTO 1

  // Create the dispatch table at function scope
  void* dispatch_table[256];

  // Dispatch macro: fetch next opcode and jump to its handler
  #define DISPATCH() do { \
    uint8_t opcode = *executor.pc++; \
    goto *dispatch_table[opcode]; \
  } while(0)

  // Jump to initialization code after all labels are defined
  goto init_dispatch_table;

  // After all opcode handlers are defined, we can safely take their addresses
init_dispatch_table:
  // Initialize all to invalid first
  for (int i = 0; i < 256; i++) {
    dispatch_table[i] = &&op_invalid;
  }

  // Set valid opcode handlers using macros for elegance and maintainability
  #define SET_DISPATCH_ENTRY_VALID(NAME) \
    dispatch_table[Util::asInteger(OpCodes::NAME)] = &&op_##NAME;
  #define SET_DISPATCH_ENTRY_INVALID(NAME)
  #define SET_DISPATCH_ENTRY(NAME, OP, VALIDITY) \
    SET_DISPATCH_ENTRY_##VALIDITY(NAME)

  ITERATE_ALL_OPCODE(SET_DISPATCH_ENTRY)

  #undef SET_DISPATCH_ENTRY
  #undef SET_DISPATCH_ENTRY_INVALID
  #undef SET_DISPATCH_ENTRY_VALID

  // Start execution - fetch first opcode
  goto *dispatch_table[*executor.pc++];

  // ===== Opcode Handlers =====
  // Each handler executes the instruction and dispatches to the next one

  op_Unreachable:
    doUnreachable(executor, std::nullopt);
    DISPATCH();

  op_Nop:
    // Inline nop for maximum performance
    DISPATCH();

  op_Block:
    doBlock(executor, std::nullopt);
    DISPATCH();

  op_Loop:
    doLoop(executor, std::nullopt);
    DISPATCH();

  op_If:
    doIf(executor, std::nullopt);
    DISPATCH();

  op_Else:
    doElse(executor, std::nullopt);
    DISPATCH();

  op_End:
    doEnd(executor, std::nullopt);
    if (executor.getCurrentStatus() != Executor::EngineStatus::EXECUTING) {
      return;  // Engine stopped
    }
    DISPATCH();

  op_Br:
    doBr(executor, std::nullopt);
    DISPATCH();

  op_BrIf:
    doBrIf(executor, std::nullopt);
    DISPATCH();

  op_BrTable:
    doBrTable(executor, std::nullopt);
    DISPATCH();

  op_Return:
    doReturn(executor, std::nullopt);
    if (executor.getCurrentStatus() != Executor::EngineStatus::EXECUTING) {
      return;  // Engine stopped
    }
    DISPATCH();

  op_Call:
    doCall(executor, std::nullopt);
    DISPATCH();

  op_CallIndirect:
    doCallIndirect(executor, std::nullopt);
    DISPATCH();

  op_Drop:
    doDrop(executor, std::nullopt);
    DISPATCH();

  op_Select:
    doSelect(executor, std::nullopt);
    DISPATCH();

  op_LocalGet:
    doLocalGet(executor, std::nullopt);
    DISPATCH();

  op_LocalSet:
    doLocalSet(executor, std::nullopt);
    DISPATCH();

  op_LocalTee:
    doLocalTee(executor, std::nullopt);
    DISPATCH();

  op_GlobalGet:
    doGlobalGet(executor, std::nullopt);
    DISPATCH();

  op_GlobalSet:
    doGlobalSet(executor, std::nullopt);
    DISPATCH();

  op_I32LoadMem:
    doI32LoadMem(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem:
    doI64LoadMem(executor, std::nullopt);
    DISPATCH();

  op_F32LoadMem:
    doF32LoadMem(executor, std::nullopt);
    DISPATCH();

  op_F64LoadMem:
    doF64LoadMem(executor, std::nullopt);
    DISPATCH();

  op_I32LoadMem8S:
    doI32LoadMem8S(executor, std::nullopt);
    DISPATCH();

  op_I32LoadMem8U:
    doI32LoadMem8U(executor, std::nullopt);
    DISPATCH();

  op_I32LoadMem16S:
    doI32LoadMem16S(executor, std::nullopt);
    DISPATCH();

  op_I32LoadMem16U:
    doI32LoadMem16U(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem8S:
    doI64LoadMem8S(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem8U:
    doI64LoadMem8U(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem16S:
    doI64LoadMem16S(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem16U:
    doI64LoadMem16U(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem32S:
    doI64LoadMem32S(executor, std::nullopt);
    DISPATCH();

  op_I64LoadMem32U:
    doI64LoadMem32U(executor, std::nullopt);
    DISPATCH();

  op_I32StoreMem:
    doI32StoreMem(executor, std::nullopt);
    DISPATCH();

  op_I64StoreMem:
    doI64StoreMem(executor, std::nullopt);
    DISPATCH();

  op_F32StoreMem:
    doF32StoreMem(executor, std::nullopt);
    DISPATCH();

  op_F64StoreMem:
    doF64StoreMem(executor, std::nullopt);
    DISPATCH();

  op_I32StoreMem8:
    doI32StoreMem8(executor, std::nullopt);
    DISPATCH();

  op_I32StoreMem16:
    doI32StoreMem16(executor, std::nullopt);
    DISPATCH();

  op_I64StoreMem8:
    doI64StoreMem8(executor, std::nullopt);
    DISPATCH();

  op_I64StoreMem16:
    doI64StoreMem16(executor, std::nullopt);
    DISPATCH();

  op_I64StoreMem32:
    doI64StoreMem32(executor, std::nullopt);
    DISPATCH();

  op_MemorySize:
    doMemorySize(executor, std::nullopt);
    DISPATCH();

  op_MemoryGrow:
    doMemoryGrow(executor, std::nullopt);
    DISPATCH();

  op_I32Const:
    doI32Const(executor, std::nullopt);
    DISPATCH();

  op_I64Const:
    doI64Const(executor, std::nullopt);
    DISPATCH();

  op_F32Const:
    doF32Const(executor, std::nullopt);
    DISPATCH();

  op_F64Const:
    doF64Const(executor, std::nullopt);
    DISPATCH();

  op_I32Eqz:
    doI32Eqz(executor, std::nullopt);
    DISPATCH();

  op_I32Eq:
    doI32Eq(executor, std::nullopt);
    DISPATCH();

  op_I32Ne:
    doI32Ne(executor, std::nullopt);
    DISPATCH();

  op_I32LtS:
    doI32LtS(executor, std::nullopt);
    DISPATCH();

  op_I32LtU:
    doI32LtU(executor, std::nullopt);
    DISPATCH();

  op_I32GtS:
    doI32GtS(executor, std::nullopt);
    DISPATCH();

  op_I32GtU:
    doI32GtU(executor, std::nullopt);
    DISPATCH();

  op_I32LeS:
    doI32LeS(executor, std::nullopt);
    DISPATCH();

  op_I32LeU:
    doI32LeU(executor, std::nullopt);
    DISPATCH();

  op_I32GeS:
    doI32GeS(executor, std::nullopt);
    DISPATCH();

  op_I32GeU:
    doI32GeU(executor, std::nullopt);
    DISPATCH();

  op_I64Eqz:
    doI64Eqz(executor, std::nullopt);
    DISPATCH();

  op_I64Eq:
    doI64Eq(executor, std::nullopt);
    DISPATCH();

  op_I64Ne:
    doI64Ne(executor, std::nullopt);
    DISPATCH();

  op_I64LtS:
    doI64LtS(executor, std::nullopt);
    DISPATCH();

  op_I64LtU:
    doI64LtU(executor, std::nullopt);
    DISPATCH();

  op_I64GtS:
    doI64GtS(executor, std::nullopt);
    DISPATCH();

  op_I64GtU:
    doI64GtU(executor, std::nullopt);
    DISPATCH();

  op_I64LeS:
    doI64LeS(executor, std::nullopt);
    DISPATCH();

  op_I64LeU:
    doI64LeU(executor, std::nullopt);
    DISPATCH();

  op_I64GeS:
    doI64GeS(executor, std::nullopt);
    DISPATCH();

  op_I64GeU:
    doI64GeU(executor, std::nullopt);
    DISPATCH();

  op_F32Eq:
    doF32Eq(executor, std::nullopt);
    DISPATCH();

  op_F32Ne:
    doF32Ne(executor, std::nullopt);
    DISPATCH();

  op_F32Lt:
    doF32Lt(executor, std::nullopt);
    DISPATCH();

  op_F32Gt:
    doF32Gt(executor, std::nullopt);
    DISPATCH();

  op_F32Le:
    doF32Le(executor, std::nullopt);
    DISPATCH();

  op_F32Ge:
    doF32Ge(executor, std::nullopt);
    DISPATCH();

  op_F64Eq:
    doF64Eq(executor, std::nullopt);
    DISPATCH();

  op_F64Ne:
    doF64Ne(executor, std::nullopt);
    DISPATCH();

  op_F64Lt:
    doF64Lt(executor, std::nullopt);
    DISPATCH();

  op_F64Gt:
    doF64Gt(executor, std::nullopt);
    DISPATCH();

  op_F64Le:
    doF64Le(executor, std::nullopt);
    DISPATCH();

  op_F64Ge:
    doF64Ge(executor, std::nullopt);
    DISPATCH();

  op_I32Clz:
    doI32Clz(executor, std::nullopt);
    DISPATCH();

  op_I32Ctz:
    doI32Ctz(executor, std::nullopt);
    DISPATCH();

  op_I32Popcnt:
    doI32Popcnt(executor, std::nullopt);
    DISPATCH();

  op_I32Add:
    doI32Add(executor, std::nullopt);
    DISPATCH();

  op_I32Sub:
    doI32Sub(executor, std::nullopt);
    DISPATCH();

  op_I32Mul:
    doI32Mul(executor, std::nullopt);
    DISPATCH();

  op_I32DivS:
    doI32DivS(executor, std::nullopt);
    DISPATCH();

  op_I32DivU:
    doI32DivU(executor, std::nullopt);
    DISPATCH();

  op_I32RemS:
    doI32RemS(executor, std::nullopt);
    DISPATCH();

  op_I32RemU:
    doI32RemU(executor, std::nullopt);
    DISPATCH();

  op_I32And:
    doI32And(executor, std::nullopt);
    DISPATCH();

  op_I32Or:
    doI32Or(executor, std::nullopt);
    DISPATCH();

  op_I32Xor:
    doI32Xor(executor, std::nullopt);
    DISPATCH();

  op_I32Shl:
    doI32Shl(executor, std::nullopt);
    DISPATCH();

  op_I32ShrS:
    doI32ShrS(executor, std::nullopt);
    DISPATCH();

  op_I32ShrU:
    doI32ShrU(executor, std::nullopt);
    DISPATCH();

  op_I32Rotl:
    doI32Rotl(executor, std::nullopt);
    DISPATCH();

  op_I32Rotr:
    doI32Rotr(executor, std::nullopt);
    DISPATCH();

  op_I64Clz:
    doI64Clz(executor, std::nullopt);
    DISPATCH();

  op_I64Ctz:
    doI64Ctz(executor, std::nullopt);
    DISPATCH();

  op_I64Popcnt:
    doI64Popcnt(executor, std::nullopt);
    DISPATCH();

  op_I64Add:
    doI64Add(executor, std::nullopt);
    DISPATCH();

  op_I64Sub:
    doI64Sub(executor, std::nullopt);
    DISPATCH();

  op_I64Mul:
    doI64Mul(executor, std::nullopt);
    DISPATCH();

  op_I64DivS:
    doI64DivS(executor, std::nullopt);
    DISPATCH();

  op_I64DivU:
    doI64DivU(executor, std::nullopt);
    DISPATCH();

  op_I64RemS:
    doI64RemS(executor, std::nullopt);
    DISPATCH();

  op_I64RemU:
    doI64RemU(executor, std::nullopt);
    DISPATCH();

  op_I64And:
    doI64And(executor, std::nullopt);
    DISPATCH();

  op_I64Or:
    doI64Or(executor, std::nullopt);
    DISPATCH();

  op_I64Xor:
    doI64Xor(executor, std::nullopt);
    DISPATCH();

  op_I64Shl:
    doI64Shl(executor, std::nullopt);
    DISPATCH();

  op_I64ShrS:
    doI64ShrS(executor, std::nullopt);
    DISPATCH();

  op_I64ShrU:
    doI64ShrU(executor, std::nullopt);
    DISPATCH();

  op_I64Rotl:
    doI64Rotl(executor, std::nullopt);
    DISPATCH();

  op_I64Rotr:
    doI64Rotr(executor, std::nullopt);
    DISPATCH();

  op_F32Abs:
    doF32Abs(executor, std::nullopt);
    DISPATCH();

  op_F32Neg:
    doF32Neg(executor, std::nullopt);
    DISPATCH();

  op_F32Ceil:
    doF32Ceil(executor, std::nullopt);
    DISPATCH();

  op_F32Floor:
    doF32Floor(executor, std::nullopt);
    DISPATCH();

  op_F32Trunc:
    doF32Trunc(executor, std::nullopt);
    DISPATCH();

  op_F32NearestInt:
    doF32NearestInt(executor, std::nullopt);
    DISPATCH();

  op_F32Sqrt:
    doF32Sqrt(executor, std::nullopt);
    DISPATCH();

  op_F32Add:
    doF32Add(executor, std::nullopt);
    DISPATCH();

  op_F32Sub:
    doF32Sub(executor, std::nullopt);
    DISPATCH();

  op_F32Mul:
    doF32Mul(executor, std::nullopt);
    DISPATCH();

  op_F32Div:
    doF32Div(executor, std::nullopt);
    DISPATCH();

  op_F32Min:
    doF32Min(executor, std::nullopt);
    DISPATCH();

  op_F32Max:
    doF32Max(executor, std::nullopt);
    DISPATCH();

  op_F32CopySign:
    doF32CopySign(executor, std::nullopt);
    DISPATCH();

  op_F64Abs:
    doF64Abs(executor, std::nullopt);
    DISPATCH();

  op_F64Neg:
    doF64Neg(executor, std::nullopt);
    DISPATCH();

  op_F64Ceil:
    doF64Ceil(executor, std::nullopt);
    DISPATCH();

  op_F64Floor:
    doF64Floor(executor, std::nullopt);
    DISPATCH();

  op_F64Trunc:
    doF64Trunc(executor, std::nullopt);
    DISPATCH();

  op_F64NearestInt:
    doF64NearestInt(executor, std::nullopt);
    DISPATCH();

  op_F64Sqrt:
    doF64Sqrt(executor, std::nullopt);
    DISPATCH();

  op_F64Add:
    doF64Add(executor, std::nullopt);
    DISPATCH();

  op_F64Sub:
    doF64Sub(executor, std::nullopt);
    DISPATCH();

  op_F64Mul:
    doF64Mul(executor, std::nullopt);
    DISPATCH();

  op_F64Div:
    doF64Div(executor, std::nullopt);
    DISPATCH();

  op_F64Min:
    doF64Min(executor, std::nullopt);
    DISPATCH();

  op_F64Max:
    doF64Max(executor, std::nullopt);
    DISPATCH();

  op_F64CopySign:
    doF64CopySign(executor, std::nullopt);
    DISPATCH();

  op_I32TruncF32S:
    doI32TruncF32S(executor, std::nullopt);
    DISPATCH();

  op_I32TruncF32U:
    doI32TruncF32U(executor, std::nullopt);
    DISPATCH();

  op_I32TruncF64S:
    doI32TruncF64S(executor, std::nullopt);
    DISPATCH();

  op_I32TruncF64U:
    doI32TruncF64U(executor, std::nullopt);
    DISPATCH();

  op_I64TruncF32S:
    doI64TruncF32S(executor, std::nullopt);
    DISPATCH();

  op_I64TruncF32U:
    doI64TruncF32U(executor, std::nullopt);
    DISPATCH();

  op_I64TruncF64S:
    doI64TruncF64S(executor, std::nullopt);
    DISPATCH();

  op_I64TruncF64U:
    doI64TruncF64U(executor, std::nullopt);
    DISPATCH();

  op_I64ExtendI32S:
    doI64ExtendI32S(executor, std::nullopt);
    DISPATCH();

  op_I64ExtendI32U:
    doI64ExtendI32U(executor, std::nullopt);
    DISPATCH();

  op_F32SConvertI32:
    doF32SConvertI32(executor, std::nullopt);
    DISPATCH();

  op_F32UConvertI32:
    doF32UConvertI32(executor, std::nullopt);
    DISPATCH();

  op_F32SConvertI64:
    doF32SConvertI64(executor, std::nullopt);
    DISPATCH();

  op_F32UConvertI64:
    doF32UConvertI64(executor, std::nullopt);
    DISPATCH();

  op_F64SConvertI32:
    doF64SConvertI32(executor, std::nullopt);
    DISPATCH();

  op_F64UConvertI32:
    doF64UConvertI32(executor, std::nullopt);
    DISPATCH();

  op_F64SConvertI64:
    doF64SConvertI64(executor, std::nullopt);
    DISPATCH();

  op_F64UConvertI64:
    doF64UConvertI64(executor, std::nullopt);
    DISPATCH();

  op_I32WrapI64:
    doI32WrapI64(executor, std::nullopt);
    DISPATCH();

  op_F32DemoteF64:
    doF32DemoteF64(executor, std::nullopt);
    DISPATCH();

  op_F64PromoteF32:
    doF64PromoteF32(executor, std::nullopt);
    DISPATCH();

  op_I32ReinterpretF32:
    doI32ReinterpretF32(executor, std::nullopt);
    DISPATCH();

  op_I64ReinterpretF64:
    doI64ReinterpretF64(executor, std::nullopt);
    DISPATCH();

  op_F32ReinterpretI32:
    doF32ReinterpretI32(executor, std::nullopt);
    DISPATCH();

  op_F64ReinterpretI64:
    doF64ReinterpretI64(executor, std::nullopt);
    DISPATCH();

  // Invalid opcode handler
  op_invalid:
    Exception::terminate(Exception::ErrorType::UNREACHABLE);
    return;

  #undef DISPATCH
  #undef USE_COMPUTED_GOTO

#else
  // Fallback for compilers without computed goto support
  #warning "Computed goto not supported by this compiler, falling back to slower dispatch"
  while (executor.getCurrentStatus() == Executor::EngineStatus::EXECUTING) {
    Interpreter::opTokenHandlers[*executor.pc++](executor, std::nullopt);
  }
#endif
}

}  // namespace TWVM
