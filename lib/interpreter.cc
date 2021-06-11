#include <array>
#include <iostream>
#include <algorithm>
#include "lib/interpreter.h"
#include "lib/structs.h"
#include "lib/executor.h"
#include "lib/exception.h"

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
#define CONCATENATE_PREFIX(X) Runtime:: X
#define DECLARE_BASIC_BINOP_METHOD(NAME, VAL_TYPE, RET_TYPE, OP_CAST, OP) \
  void Interpreter::do##NAME(Executor& executor) { \
    executor.opHelperFTTO<CONCATENATE_PREFIX(VAL_TYPE), CONCATENATE_PREFIX(RET_TYPE)>([](auto x, auto y) { \
      return static_cast<CONCATENATE_PREFIX(OP_CAST)>(x) OP static_cast<CONCATENATE_PREFIX(OP_CAST)>(y); \
    }); \
  }
  
namespace TWVM {
  std::array<Interpreter::opHandlerProto, sizeof(uint8_t) * 1 << 8> Interpreter::opTokenHandlers = {
    ITERATE_ALL_OPCODE(REF_OPCODE_HANDLER_PTR)
  };

  void Interpreter::doUnreachable(Executor& executor) {
    Exception::terminate(Exception::ErrorType::UNREACHABLE);
  }
  void Interpreter::doNop(Executor& executor) {
    executor.movPC();
  }
  void Interpreter::doBlock(Executor& executor) {
    auto&& returnArityTypes = std::vector<uint8_t>{}; // at most one.
    const auto returnTypeByte = executor.decodeByteFromPC();
    if (static_cast<LangTypes>(returnTypeByte) != LangTypes::Void) {
      returnArityTypes.push_back(returnTypeByte);
    }
    auto cont = executor.lookupLabelContFromPC();
    executor.pushToStack(Runtime::RTLabelFrame(cont, returnArityTypes));
  }
  void Interpreter::doLoop(Executor& executor) {
    
  }
  void Interpreter::doIf(Executor& executor) {
    
  }
  void Interpreter::doElse(Executor& executor) {
    
  }
  void Interpreter::doEnd(Executor& executor) {
    std::cout << 11;
  }
  void Interpreter::doBr(Executor& executor) {
    const auto depth = executor.getBrIfDepthCacheOr(executor.decodeVaruintFromPC<Runtime::relative_depth_t>());
    const auto labelsCount = executor.getLabelAboveActivFrameCount();
    if (labelsCount > depth) {  
      // Consume Label frames.
      const auto& frameOffset = executor.refTopFrameByType(Runtime::STVariantIndex::LABEL, depth);
      const auto& labelFrame = std::get<Runtime::RTLabelFrame>(*((*frameOffset).ptr));
      const auto& returnArity = labelFrame.returnArity;
      if (returnArity.size() > 0) {
        executor.validateArity(returnArity);  // May throw.
      }
      executor.eraseFromStack(frameOffset->offset, returnArity.size());
      executor.eraseFromFrameBitmap(Runtime::STVariantIndex::LABEL, depth + 1);
      executor.setPC(labelFrame.cont);
    } else if (labelsCount == depth) {  
      // Consuem Activ frame.
      const auto& frameOffset = executor.refTopFrameByType(Runtime::STVariantIndex::ACTIVATION, depth);
      const auto& labelActiv = std::get<Runtime::RTLabelFrame>(*((*frameOffset).ptr));
      const auto& returnArity = labelActiv.returnArity;
      if (returnArity.size() > 0) {
        executor.validateArity(returnArity);
      }
      executor.eraseFromStack(frameOffset->offset, returnArity.size());
      executor.eraseFromFrameBitmap(Runtime::STVariantIndex::LABEL, depth);
      executor.eraseFromFrameBitmap(Runtime::STVariantIndex::ACTIVATION, 1);
      executor.setPC(labelActiv.cont);
    } else {
      Exception::terminate(Exception::ErrorType::BREAK_LEVEL_EXCEEDED);
    }
    executor.delBrIfDepthCache();
  }
  void Interpreter::doBrIf(Executor& executor) {
    const auto v = executor.popValFromStack<Runtime::rt_i32_t>();
    const auto depth = executor.decodeVaruintFromPC<Runtime::relative_depth_t>();
    if (v != 0) {
      executor.setBrIfDepthCache(depth);
      doBr(executor);
    }
  }
  void Interpreter::doBrTable(Executor& executor) {
    
  }
  void Interpreter::doReturn(Executor& executor) {
    std::cout << 11;
  }
  void Interpreter::doCall(Executor& executor) {
    // Retrieve func idx.
    const auto& funcIdx = executor.decodeVaruintFromPC<Runtime::index_t>();
    const auto& rtFuncDesc = executor.getEngineData()->rtFuncDescriptor.at(funcIdx);
    // Retrieve func related info.
    auto paramCount = rtFuncDesc.funcType->first.size();
    auto rtFuncLocals = rtFuncDesc.localsDefault;  // Copied.
    // Set up func parameters.
    if (paramCount > 0) {
      for (auto i = 0; i < paramCount; ++i) {
        const auto& vf = executor.retrieveFromStack<Runtime::RTValueFrame>();
        if (vf.value.index() == rtFuncLocals.at(i).index()) {
          rtFuncLocals.at(i) = vf.value;
          executor.popFromStack();
        } else {
          Exception::terminate(Exception::ErrorType::MISSING_FUNC_PARAMS);
        }
      }
    }
    // Construct frame (locals + artiy).
    executor.pushToStack(
      Runtime::RTActivFrame(
        rtFuncLocals, 
        executor.movPC(), 
        &rtFuncDesc.funcType->second));
    // Redirection.
    executor.setPC(rtFuncDesc.codeEntry);
  }
  void Interpreter::doCallIndirect(Executor& executor) {
    
  }
  void Interpreter::doDrop(Executor& executor) {
    
  }
  void Interpreter::doSelect(Executor& executor) {
    
  }
  void Interpreter::doLocalGet(Executor& executor) {
    const auto& idx = executor.decodeVaruintFromPC<Runtime::index_t>();
    const auto frameOffset = executor.refTopFrameByType(Runtime::STVariantIndex::ACTIVATION);
    if (!frameOffset.has_value()) {
      Exception::terminate(Exception::ErrorType::EXHAUSTED_STACK_ACCESS);
    } else {
      auto& topActivFrame = *((*frameOffset).ptr);
      const auto& locals = std::get<Runtime::RTActivFrame>(topActivFrame).locals;
      if (locals.size() >= idx + 1) {
        executor.pushToStack(locals.at(idx));
      } else {
        Exception::terminate(Exception::ErrorType::ILLEGAL_LOCAL_IDX);
      }
    }
  }
  void Interpreter::doLocalSet(Executor& executor) {
    
  }
  void Interpreter::doLocalTee(Executor& executor) {
    
  }
  void Interpreter::doGlobalGet(Executor& executor) {
    
  }
  void Interpreter::doGlobalSet(Executor& executor) {
    
  }
  void Interpreter::doI32Const(Executor& executor) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i32_t>()));
  }
  void Interpreter::doI64Const(Executor& executor) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeVarintFromPC<Runtime::rt_i64_t>()));
  }
  void Interpreter::doF32Const(Executor& executor) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<float>()));
  }
  void Interpreter::doF64Const(Executor& executor) {
    executor.pushToStack(Runtime::RTValueFrame(executor.decodeFloatingPointFromPC<double>()));
  }
  void Interpreter::doI32LoadMem(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem(Executor& executor) {
    
  }
  void Interpreter::doF32LoadMem(Executor& executor) {
    
  }
  void Interpreter::doF64LoadMem(Executor& executor) {
    
  }
  void Interpreter::doI32LoadMem8S(Executor& executor) {
    
  }
  void Interpreter::doI32LoadMem8U(Executor& executor) {
    
  }
  void Interpreter::doI32LoadMem16S(Executor& executor) {
    
  }
  void Interpreter::doI32LoadMem16U(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem8S(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem8U(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem16S(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem16U(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem32S(Executor& executor) {
    
  }
  void Interpreter::doI64LoadMem32U(Executor& executor) {
    
  }
  void Interpreter::doI32StoreMem(Executor& executor) {
    
  }
  void Interpreter::doI64StoreMem(Executor& executor) {
    
  }
  void Interpreter::doF32StoreMem(Executor& executor) {
    
  }
  void Interpreter::doF64StoreMem(Executor& executor) {
    
  }
  void Interpreter::doI32StoreMem8(Executor& executor) {
    
  }
  void Interpreter::doI32StoreMem16(Executor& executor) {
    
  }
  void Interpreter::doI64StoreMem8(Executor& executor) {
    
  }
  void Interpreter::doI64StoreMem16(Executor& executor) {
    
  }
  void Interpreter::doI64StoreMem32(Executor& executor) {
    
  }
  void Interpreter::doMemorySize(Executor& executor) {
    
  }
  void Interpreter::doMemoryGrow(Executor& executor) {
    
  }
  
  ITERATE_SIMPLE_BINOP(DECLARE_BASIC_BINOP_METHOD)

  void Interpreter::doI32Eqz(Executor& executor) {
    
  }
  void Interpreter::doI64Eqz(Executor& executor) {
    
  }
  void Interpreter::doI32Clz(Executor& executor) {
    
  }
  void Interpreter::doI32Ctz(Executor& executor) {
    
  }
  void Interpreter::doI32Popcnt(Executor& executor) {
    
  }
  void Interpreter::doI32DivS(Executor& executor) {
    
  }
  void Interpreter::doI32DivU(Executor& executor) {
    
  }
  void Interpreter::doI32RemS(Executor& executor) {
    
  }
  void Interpreter::doI32RemU(Executor& executor) {
    
  }
  void Interpreter::doI32Shl(Executor& executor) {
    
  }
  void Interpreter::doI32ShrS(Executor& executor) {
    
  }
  void Interpreter::doI32ShrU(Executor& executor) {
    
  }
  void Interpreter::doI32Rotl(Executor& executor) {
    
  }
  void Interpreter::doI32Rotr(Executor& executor) {
    
  }
  void Interpreter::doI64Clz(Executor& executor) {
    
  }
  void Interpreter::doI64Ctz(Executor& executor) {
    
  }
  void Interpreter::doI64Popcnt(Executor& executor) {
    
  }
  void Interpreter::doI64DivS(Executor& executor) {
    
  }
  void Interpreter::doI64DivU(Executor& executor) {
    
  }
  void Interpreter::doI64RemS(Executor& executor) {
    
  }
  void Interpreter::doI64RemU(Executor& executor) {
    
  }
  void Interpreter::doI64Shl(Executor& executor) {
    
  }
  void Interpreter::doI64ShrS(Executor& executor) {
    
  }
  void Interpreter::doI64ShrU(Executor& executor) {
    
  }
  void Interpreter::doI64Rotl(Executor& executor) {
    
  }
  void Interpreter::doI64Rotr(Executor& executor) {
    
  }
  void Interpreter::doF32Abs(Executor& executor) {
    
  }
  void Interpreter::doF32Neg(Executor& executor) {
    
  }
  void Interpreter::doF32Ceil(Executor& executor) {
    
  }
  void Interpreter::doF32Floor(Executor& executor) {
    
  }
  void Interpreter::doF32Trunc(Executor& executor) {
    
  }
  void Interpreter::doF32NearestInt(Executor& executor) {
    
  }
  void Interpreter::doF32Sqrt(Executor& executor) {
    
  }
  void Interpreter::doF32Min(Executor& executor) {
    
  }
  void Interpreter::doF32Max(Executor& executor) {
    
  }
  void Interpreter::doF32CopySign(Executor& executor) {
    
  }
  void Interpreter::doF64Abs(Executor& executor) {
    
  }
  void Interpreter::doF64Neg(Executor& executor) {
    
  }
  void Interpreter::doF64Ceil(Executor& executor) {
    
  }
  void Interpreter::doF64Floor(Executor& executor) {
    
  }
  void Interpreter::doF64Trunc(Executor& executor) {
    
  }
  void Interpreter::doF64NearestInt(Executor& executor) {
    
  }
  void Interpreter::doF64Sqrt(Executor& executor) {
    
  }
  void Interpreter::doF64Min(Executor& executor) {
    
  }
  void Interpreter::doF64Max(Executor& executor) {
    
  }
  void Interpreter::doF64CopySign(Executor& executor) {
    
  }
  void Interpreter::doI32WrapI64(Executor& executor) {
    
  }
  void Interpreter::doI32TruncF32S(Executor& executor) {
    
  }
  void Interpreter::doI32TruncF32U(Executor& executor) {
    
  }
  void Interpreter::doI32TruncF64S(Executor& executor) {
    
  }
  void Interpreter::doI32TruncF64U(Executor& executor) {
    
  }
  void Interpreter::doI64ExtendI32S(Executor& executor) {
    
  }
  void Interpreter::doI64ExtendI32U(Executor& executor) {
    
  }
  void Interpreter::doI64TruncF32S(Executor& executor) {
    
  }
  void Interpreter::doI64TruncF32U(Executor& executor) {
    
  }
  void Interpreter::doI64TruncF64S(Executor& executor) {
    
  }
  void Interpreter::doI64TruncF64U(Executor& executor) {
    
  }
  void Interpreter::doF32SConvertI32(Executor& executor) {
    
  }
  void Interpreter::doF32UConvertI32(Executor& executor) {
    
  }
  void Interpreter::doF32SConvertI64(Executor& executor) {
    
  }
  void Interpreter::doF32UConvertI64(Executor& executor) {
    
  }
  void Interpreter::doF32DemoteF64(Executor& executor) {
    
  }
  void Interpreter::doF64SConvertI32(Executor& executor) {
    
  }
  void Interpreter::doF64UConvertI32(Executor& executor) {
    
  }
  void Interpreter::doF64SConvertI64(Executor& executor) {
    
  }
  void Interpreter::doF64UConvertI64(Executor& executor) {
    
  }
  void Interpreter::doF64PromoteF32(Executor& executor) {
    
  }
  void Interpreter::doI32ReinterpretF32(Executor& executor) {
    
  }
  void Interpreter::doI64ReinterpretF64(Executor& executor) {
    
  }
  void Interpreter::doF32ReinterpretI32(Executor& executor) {
    
  }
  void Interpreter::doF64ReinterpretI64(Executor& executor) {
    
  }
}
