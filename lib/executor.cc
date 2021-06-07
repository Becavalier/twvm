#include "lib/executor.h"
#include "lib/interpreter.h"

namespace TWVM {
  void Executor::execute(shared_module_instance_t rtIns) {
    if (rtIns->rtEntryIdx.has_value()) {
      Executor executor(rtIns->module->funcDefs.at(*rtIns->rtEntryIdx).body.data());
      Interpreter::opTokenHandlers[*executor.pc++](executor, rtIns);
    } 
  }
}
