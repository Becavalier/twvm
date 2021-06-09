#include <optional>
#include "lib/executor.h"
#include "lib/decoder.h"
#include "lib/interpreter.h"
#include "lib/opcodes.h"
#include "lib/util.h"

namespace TWVM {
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
      while (executor.currentStatus() == Executor::EngineStatus::RUNNING) {
        Interpreter::opTokenHandlers[*executor.pc++](executor);
      }
    }
  }
}
