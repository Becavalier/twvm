#include <optional>
#include "lib/executor.h"
#include "lib/decoder.h"
#include "lib/interpreter.h"
#include "lib/opcodes.h"
#include "lib/util.h"

namespace TWVM {
  std::optional<uint32_t> Executor::getTopFrameIdx(Runtime::STVariantIndex type, uint32_t n) {
    const auto& v = frameBitmap.at(Util::asInteger(type));
    return v.size() > n ? 
      std::make_optional(*(v.rbegin() + n)) : 
      std::nullopt;
  }
  std::optional<Executor::FrameOffset> Executor::refTopFrameByType(Runtime::STVariantIndex type, uint32_t n) {
    const auto topIdx = getTopFrameIdx(type, n);
    return topIdx.has_value() ? 
      std::make_optional(
        FrameOffset {
          &rtIns->stack.at(*topIdx), *topIdx,
        }) : std::nullopt;
  }
  uint8_t* Executor::lookupLabelContFromPC() {  // don't mess this process with interpreter.
    status = EngineStatus::CRAWLING;
    storedPC = pc;
    size_t pairingCount = 0;
    while (status == EngineStatus::CRAWLING) {
      Executor::crawlOpCodes(pairingCount);
    }
    const auto cont = pc;
    pc = storedPC;
    return cont;
  }
  void Executor::crawlOpCodes(size_t pairingCount) {
    switch (static_cast<OpCodes>(*pc++)) {
      case OpCodes::Block:
      case OpCodes::Loop:
      case OpCodes::If: {
        decodeByteFromPC();
        pairingCount++;
        break;
      }
      case OpCodes::Br:
      case OpCodes::BrIf: {
        decodeVaruintFromPC<Runtime::relative_depth_t>(); 
        break;
      }
      case OpCodes::BrTable: {
        const auto targetCount = decodeVaruintFromPC<uint32_t>();
        for (uint32_t i = 0; i < targetCount + 1; ++i) {  // Include `default_target`.
          decodeVaruintFromPC<uint32_t>();
        }
        break;
      }
      case OpCodes::Call: {
        decodeVaruintFromPC<Runtime::index_t>(); 
        break;
      }
      case OpCodes::CallIndirect: {
        decodeVaruintFromPC<Runtime::index_t>();
        decodeByteFromPC();
        break;
      }
      case OpCodes::LocalGet:
      case OpCodes::LocalSet:
      case OpCodes::LocalTee:
      case OpCodes::GlobalGet:
      case OpCodes::GlobalSet: {
        decodeVaruintFromPC<Runtime::index_t>(); 
        break;
      }
      case OpCodes::I32LoadMem:
      case OpCodes::I64LoadMem:
      case OpCodes::F32LoadMem:
      case OpCodes::F64LoadMem:
      case OpCodes::I32LoadMem8S:
      case OpCodes::I32LoadMem8U:
      case OpCodes::I32LoadMem16S:
      case OpCodes::I32LoadMem16U:
      case OpCodes::I64LoadMem8S:
      case OpCodes::I64LoadMem8U:
      case OpCodes::I64LoadMem16S:
      case OpCodes::I64LoadMem16U:
      case OpCodes::I64LoadMem32S:
      case OpCodes::I64LoadMem32U:
      case OpCodes::I32StoreMem:
      case OpCodes::I64StoreMem:
      case OpCodes::F32StoreMem:
      case OpCodes::F64StoreMem:
      case OpCodes::I32StoreMem8:
      case OpCodes::I32StoreMem16:
      case OpCodes::I64StoreMem8:
      case OpCodes::I64StoreMem16:
      case OpCodes::I64StoreMem32: {
        decodeVaruintFromPC<Runtime::index_t>();
        decodeVaruintFromPC<Runtime::index_t>();
        break;
      }
      case OpCodes::I32Const: {
        decodeVaruintFromPC<Runtime::rt_i32_t>();
        break;
      }
      case OpCodes::I64Const: {
        decodeVaruintFromPC<Runtime::rt_i64_t>();
        break;
      }
      case OpCodes::F32Const: {
        decodeFloatingPointFromPC<float>();
        break;
      }
      case OpCodes::F64Const: {
        decodeFloatingPointFromPC<double>();
        break;
      }
      case OpCodes::End: {
        if (pairingCount == 0) {
          status = EngineStatus::EXECUTING;
        } else {
          pairingCount--;
        }
        break;
      }
      default: break;
    }
  }
  void Executor::execute(
    shared_module_runtime_t rtIns, 
    std::optional<uint32_t> invokeIdx) {
    if (!invokeIdx.has_value()) {
      if (rtIns->rtEntryIdx.has_value()) {  // invoke `main`.
        invokeIdx = *rtIns->rtEntryIdx;
      } 
    }
    if (invokeIdx.has_value()) {
      // [CALL, (IDX), END].
      std::vector<uint8_t> driver = { Util::asInteger(OpCodes::Call) };  // driver opcodes.
      const auto bytes = Decoder::encodeVaruint(*invokeIdx);
      driver.insert(driver.end(), bytes.begin(), bytes.end());
      driver.push_back(Util::asInteger(OpCodes::End));
      Executor executor(driver.data(), rtIns);
      while (executor.getCurrentStatus() == Executor::EngineStatus::EXECUTING) {
        Interpreter::opTokenHandlers[*executor.pc++](executor);
      }
    }
  }
}
