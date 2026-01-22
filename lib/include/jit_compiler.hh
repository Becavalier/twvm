// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_JIT_COMPILER_HH_
#define LIB_INCLUDE_JIT_COMPILER_HH_

#include <memory>
#include <string>
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "lib/include/structs.hh"

namespace TWVM {

class JITCompiler {
 public:
  // Singleton pattern for global JIT instance
  static JITCompiler& getInstance();

  // Initialize LLVM ORC JIT engine
  bool initialize();

  // Compile a WebAssembly function to native code
  bool compileFunction(uint32_t funcIdx, Runtime::RTFuncDescriptor& descriptor,
                      shared_module_runtime_t rtIns);

  // Lookup compiled function pointer
  void* lookupCompiledFunction(const std::string& funcName);

  // Check if JIT is initialized
  bool isInitialized() const { return jitInitialized; }

 private:
  JITCompiler();
  ~JITCompiler();

  // Prevent copying
  JITCompiler(const JITCompiler&) = delete;
  JITCompiler& operator=(const JITCompiler&) = delete;

  // LLVM JIT infrastructure
  std::unique_ptr<llvm::orc::LLJIT> jit;
  std::unique_ptr<llvm::LLVMContext> context;
  bool jitInitialized = false;

  // Translate WebAssembly bytecode to LLVM IR
  llvm::Function* translateToIR(uint32_t funcIdx,
                                const Runtime::RTFuncDescriptor& descriptor,
                                shared_module_runtime_t rtIns,
                                llvm::Module* module);

  // Helper: Create LLVM function signature from WebAssembly function type
  llvm::FunctionType* createFunctionType(const Module::func_type_t& funcType,
                                         llvm::LLVMContext& ctx);
};

}  // namespace TWVM

#endif  // LIB_INCLUDE_JIT_COMPILER_HH_
