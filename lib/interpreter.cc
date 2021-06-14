#include <array>
#include <iostream>
#include <algorithm>
#include "lib/interpreter.h"
#include "lib/structs.h"
#include "lib/executor.h"
#include "lib/exception.h"
#include "lib/opcodes.h"

#define ITERATE_SIMPLE_BINOP(V) \
  V(I32Mul, rt_i32_t, rt_i32_t, rt_i32_t, *) \
  V(I32Add, rt_i32_t, rt_i32_t, rt_i32_t, +) \
  V(I32Sub, rt_i32_t, rt_i32_t, rt_i32_t, -) \
  V(I32And, rt_i32_t, rt_i32_t, rt_i32_t, &) \
  V(I32Or, rt_i32_t, rt_i32_t, rt_i32_t, |) \
  V(I32Xor, rt_i32_t, rt_i32_t, rt_i32_t, ^) \
  V(I32Eq, rt_i32_t, rt_i32_t, rt_i32_t, ==) \
  V(I32Ne, rt_i32_t, rt_i32_t, rt_i32_t, !=) \
  V(I32LtU, rt_i32_t, rt_i32_t, rt_u32_t, <) \
  V(I32LeU, rt_i32_t, rt_i32_t, rt_u32_t, <=) \
  V(I32GtU, rt_i32_t, rt_i32_t, rt_u32_t, >) \
  V(I32GeU, rt_i32_t, rt_i32_t, rt_u32_t, >=) \
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
  V(I64LtU, rt_i64_t, rt_i32_t, rt_u64_t, <) \
  V(I64LeU, rt_i64_t, rt_i32_t, rt_u64_t, <=) \
  V(I64GtU, rt_i64_t, rt_i32_t, rt_u64_t, >) \
  V(I64GeU, rt_i64_t, rt_i32_t, rt_u64_t, >=) \
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
#define DECLARE_BASIC_BINOP_METHOD(NAME, VAL_TYPE, RET_TYPE, OP_CAST_TYPE, OP) \
  void Interpreter::do##NAME(Executor& executor, opHandlerInfoType _) { \
    executor.opHelperFTTO<CONCAT_PREFIX(VAL_TYPE), CONCAT_PREFIX(RET_TYPE)>([](auto x, auto y) { \
      return static_cast<CONCAT_PREFIX(OP_CAST_TYPE)>(x) OP static_cast<CONCAT_PREFIX(OP_CAST_TYPE)>(y); \
    }); \
  }
  
namespace TWVM {
  std::array<Interpreter::opHandlerProto, sizeof(uint8_t) * 1 << 8> Interpreter::opTokenHandlers = {
    ITERATE_ALL_OPCODE(REF_OPCODE_HANDLER_PTR)
  };

  ITERATE_SIMPLE_BINOP(DECLARE_BASIC_BINOP_METHOD)

  void Interpreter::doUnreachable(Executor& executor, opHandlerInfoType _) {
    Exception::terminate(Exception::ErrorType::UNREACHABLE);
  }
  void Interpreter::doNop(Executor& executor, opHandlerInfoType _) {}
  void Interpreter::doBlock(Executor& executor, opHandlerInfoType _) {
    const auto returnArityTypes = executor.collectArities();
    const auto cont = executor.lookupLabelContFromPC();
    if (cont.size() > 0) {
      executor.pushToStack(Runtime::RTLabelFrame(cont.back(), returnArityTypes));
    } else  {
      Exception::terminate(Exception::ErrorType::ILLFORMED_STRUCTURE);
    }
  }
  void Interpreter::doLoop(Executor& executor, opHandlerInfoType _) {
    auto* cont = executor.getPC() - 1;
    const auto returnArityTypes = executor.collectArities();
    executor.pushToStack(Runtime::RTLabelFrame(cont));  // No arities in MVP.
  }
  void Interpreter::doIf(Executor& executor, opHandlerInfoType _) {
    const auto returnArityTypes = executor.collectArities();
    const auto conts = executor.lookupLabelContFromPC();
    if (conts.size() > 1) {
      const auto v = executor.popValFromStack<Runtime::rt_i32_t>();
      executor.pushToStack(Runtime::RTLabelFrame(conts.back(), returnArityTypes));
      if (v == 0) {
        executor.setPC(conts.front());
      }
    } else  {
      Exception::terminate(Exception::ErrorType::ILLFORMED_STRUCTURE);
    }
  }
  void Interpreter::doElse(Executor& executor, opHandlerInfoType _) {
    doBr(executor, 0);
  }
  void Interpreter::doEnd(Executor& executor, opHandlerInfoType _) {
    doBr(executor, 0);  // No forwarding PC.
  }
  void Interpreter::doBr(Executor& executor, opHandlerInfoType passedDepth) {
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
      doReturn(executor, std::make_optional(depth));
    } else {
      Exception::terminate(Exception::ErrorType::BREAK_LEVEL_EXCEEDED);
    }
  }
  void Interpreter::doBrIf(Executor& executor, opHandlerInfoType _) {
    const auto v = executor.popValFromStack<Runtime::rt_i32_t>();
    const auto depth = executor.decodeVaruintFromPC<Runtime::relative_depth_t>();
    if (v != 0) {
      doBr(executor, std::make_optional(depth));
    }
  }
  void Interpreter::doBrTable(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doReturn(Executor& executor, opHandlerInfoType labelDepth) {
    const auto activIdx = executor.getTopFrameIdx(Runtime::STVariantIndex::ACTIVATION);
    if (activIdx.has_value() && *activIdx == 0) {
      executor.stopEngine();
    } else {
      const auto depth = labelDepth.value_or(executor.getLabelAboveActivFrameCount());
      executor.setPC(
        executor.retFromFrameWithCont<Runtime::RTActivFrame>(depth));
    }
  }
  void Interpreter::doCall(Executor& executor, opHandlerInfoType _) {
    const auto idx = executor.decodeVaruintFromPC<Runtime::index_t>();
    const auto& descriptor = executor.getEngineData()->rtFuncDescriptor.at(idx);
    auto paramCount = descriptor.funcType->first.size();
    auto rtLocals = descriptor.localsDefault;  // Copied.
    // Set up func parameters.
    if (paramCount > 0) {
      for (auto i = 0; i < paramCount; ++i) {
        const auto& vf = executor.retrieveFromStack<Runtime::RTValueFrame>();
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
  void Interpreter::doCallIndirect(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doDrop(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doSelect(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doLocalGet(Executor& executor, opHandlerInfoType _) {
    const auto idx = executor.decodeVaruintFromPC<Runtime::index_t>();
    const auto& frameOffset = executor.refTrackedTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
    const auto& locals = std::get<Runtime::RTActivFrame>(*frameOffset.ptr).locals;
    if (locals.size() >= idx + 1) {
      executor.pushToStack(locals.at(idx));
    } else {
      Exception::terminate(Exception::ErrorType::ILLEGAL_LOCAL_IDX);
    }
  }
  void Interpreter::doLocalSet(Executor& executor, opHandlerInfoType fromTee) {
    const auto localIdx = executor.decodeVaruintFromPC<Runtime::index_t>();
    const auto& activFrameOffset = executor.refTrackedTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
    const auto& valueFrame = executor.refTopValFrame();
    auto& locals = std::get<Runtime::RTActivFrame>(*activFrameOffset.ptr).locals;
    if (locals.at(localIdx).index() == valueFrame.value.index()) {
      locals.at(localIdx) = valueFrame.value;
    }
    if (!fromTee.has_value()) {
      executor.popFromStack();
    }
  }
  void Interpreter::doLocalTee(Executor& executor, opHandlerInfoType _) {
    doLocalSet(executor, INFO_BOOL_TRUE);
  }
  void Interpreter::doGlobalGet(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doGlobalSet(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Const(Executor& executor, opHandlerInfoType _) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i32_t>()));
  }
  void Interpreter::doI64Const(Executor& executor, opHandlerInfoType _) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i64_t>()));
  }
  void Interpreter::doF32Const(Executor& executor, opHandlerInfoType _) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<float>()));
  }
  void Interpreter::doF64Const(Executor& executor, opHandlerInfoType _) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<double>()));
  }
  void Interpreter::doI32LoadMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32LoadMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64LoadMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32LoadMem8S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32LoadMem8U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32LoadMem16S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32LoadMem16U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem8S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem8U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem16S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem16U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem32S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64LoadMem32U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32StoreMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64StoreMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32StoreMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64StoreMem(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32StoreMem8(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32StoreMem16(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64StoreMem8(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64StoreMem16(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64StoreMem32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doMemorySize(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doMemoryGrow(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Eqz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Eqz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Clz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Ctz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Popcnt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32DivS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32DivU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32RemS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32RemU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Shl(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32ShrS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32ShrU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Rotl(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32Rotr(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Clz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Ctz(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Popcnt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64DivS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64DivU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64RemS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64RemU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Shl(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64ShrS(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64ShrU(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Rotl(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64Rotr(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Abs(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Neg(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Ceil(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Floor(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Trunc(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32NearestInt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Sqrt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Min(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32Max(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32CopySign(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Abs(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Neg(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Ceil(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Floor(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Trunc(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64NearestInt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Sqrt(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Min(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64Max(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64CopySign(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32WrapI64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32TruncF32S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32TruncF32U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32TruncF64S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32TruncF64U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64ExtendI32S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64ExtendI32U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64TruncF32S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64TruncF32U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64TruncF64S(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64TruncF64U(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32SConvertI32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32UConvertI32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32SConvertI64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32UConvertI64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32DemoteF64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64SConvertI32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64UConvertI32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64SConvertI64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64UConvertI64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64PromoteF32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI32ReinterpretF32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doI64ReinterpretF64(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF32ReinterpretI32(Executor& executor, opHandlerInfoType _) {
    
  }
  void Interpreter::doF64ReinterpretI64(Executor& executor, opHandlerInfoType _) {
    
  }
}
