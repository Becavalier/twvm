#include <optional>
#include "lib/executor.h"
#include "lib/decoder.h"
#include "lib/interpreter.h"
#include "lib/opcodes.h"
#include "lib/util.h"

namespace TWVM {
  void Executor::execute(
    shared_module_instance_t rtIns, 
    std::optional<uint32_t> invokeIdx) {
    if (!invokeIdx.has_value()) {
      if (rtIns->rtEntryIdx.has_value()) {  // invoke `main`.
        invokeIdx = *rtIns->rtEntryIdx;
      } 
    }
    if (invokeIdx.has_value()) {
      std::vector<uint8_t> driver = { Util::asInteger(OpCodes::Call) };  // driver opcodes.
      const auto bytes = Decoder::encodeVaruint(*invokeIdx);
      driver.insert(driver.end(), bytes.begin(), bytes.end());
      Executor executor(driver.data());
      while (executor.isRunning) {
        Interpreter::opTokenHandlers[*executor.pc++](executor, rtIns);
      }
    }
  }
}
