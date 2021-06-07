#include <array>
#include <iostream>
#include "lib/interpreter.h"
#include "lib/decoder.h"
#include "lib/structs.h"
#include "lib/executor.h"

#define REF_OPCODE_HANDLER_PTR_VALID(NAME) \
  Interpreter::do##NAME,
#define REF_OPCODE_HANDLER_PTR_INVALID(NAME) \
  nullptr,
#define REF_OPCODE_HANDLER_PTR(NAME, OP, VALIDITY) \
  REF_OPCODE_HANDLER_PTR_##VALIDITY(NAME)
  
namespace TWVM {
  std::array<Interpreter::opHandlerProto, sizeof(uint8_t) * 1 << 8> Interpreter::opTokenHandlers = {
    ITERATE_ALL_OPCODE(REF_OPCODE_HANDLER_PTR)
  };

  void Interpreter::doUnreachable(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doNop(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doBlock(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doLoop(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doIf(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doElse(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doEnd(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doBr(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doBrIf(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doBrTable(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doReturn(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doCall(Executor& executor, shared_module_instance_t rtIns) {
    
    std::cout << 1;
  }
  void Interpreter::doCallIndirect(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doDrop(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doSelect(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doLocalGet(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doLocalSet(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doLocalTee(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doGlobalGet(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doGlobalSet(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Const(Executor& executor, shared_module_instance_t rtIns) {
    rtIns->rtValueStack.emplace_back(Decoder::decodeVarint<RTI32>(executor.pc));
  }
  void Interpreter::doI64Const(Executor& executor, shared_module_instance_t rtIns) {
    rtIns->rtValueStack.emplace_back(Decoder::decodeVarint<RTI64>(executor.pc));
  }
  void Interpreter::doF32Const(Executor& executor, shared_module_instance_t rtIns) {

  }
  void Interpreter::doF64Const(Executor& executor, shared_module_instance_t rtIns) {

  }
  void Interpreter::doI32LoadMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32LoadMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64LoadMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LoadMem8S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LoadMem8U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LoadMem16S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LoadMem16U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem8S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem8U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem16S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem16U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem32S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LoadMem32U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32StoreMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64StoreMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32StoreMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64StoreMem(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32StoreMem8(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32StoreMem16(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64StoreMem8(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64StoreMem16(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64StoreMem32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doMemorySize(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doMemoryGrow(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Eqz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Eq(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Ne(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LtS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LtU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32GtS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32GtU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LeS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32LeU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32GeS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32GeU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Eqz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Eq(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Ne(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LtS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LtU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64GtS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64GtU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LeS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64LeU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64GeS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64GeU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Eq(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Ne(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Lt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Gt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Le(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Ge(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Eq(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Ne(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Lt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Gt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Le(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Ge(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Clz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Ctz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Popcnt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Add(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Sub(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Mul(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32DivS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32DivU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32RemS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32RemU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32And(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Or(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Xor(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Shl(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32ShrS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32ShrU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Rotl(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32Rotr(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Clz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Ctz(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Popcnt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Add(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Sub(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Mul(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64DivS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64DivU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64RemS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64RemU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64And(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Or(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Xor(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Shl(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64ShrS(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64ShrU(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Rotl(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64Rotr(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Abs(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Neg(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Ceil(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Floor(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Trunc(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32NearestInt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Sqrt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Add(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Sub(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Mul(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Div(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Min(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32Max(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32CopySign(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Abs(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Neg(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Ceil(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Floor(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Trunc(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64NearestInt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Sqrt(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Add(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Sub(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Mul(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Div(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Min(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64Max(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64CopySign(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32WrapI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32TruncF32S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32TruncF32U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32TruncF64S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32TruncF64U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64ExtendI32S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64ExtendI32U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64TruncF32S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64TruncF32U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64TruncF64S(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64TruncF64U(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32SConvertI32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32UConvertI32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32SConvertI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32UConvertI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32DemoteF64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64SConvertI32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64UConvertI32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64SConvertI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64UConvertI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64PromoteF32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI32ReinterpretF32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doI64ReinterpretF64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF32ReinterpretI32(Executor& executor, shared_module_instance_t rtIns) {
    
  }
  void Interpreter::doF64ReinterpretI64(Executor& executor, shared_module_instance_t rtIns) {
    
  }
}
