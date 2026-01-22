// Copyright 2021 YHSPY. All rights reserved.
#include "lib/include/jit_compiler.hh"
#include "lib/include/opcodes.hh"
#include "lib/include/decoder.hh"
#include "lib/include/util.hh"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include <iostream>

namespace TWVM {

JITCompiler& JITCompiler::getInstance() {
  static JITCompiler instance;
  return instance;
}

JITCompiler::JITCompiler() {}

JITCompiler::~JITCompiler() {}

bool JITCompiler::initialize() {
  if (jitInitialized) {
    return true;
  }

  // Initialize LLVM native target
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Create LLVM context
  context = std::make_unique<llvm::LLVMContext>();

  // Create LLJIT instance
  auto jitOrErr = llvm::orc::LLJITBuilder().create();
  if (!jitOrErr) {
    llvm::errs() << "Failed to create LLJIT: " << llvm::toString(jitOrErr.takeError()) << "\n";
    return false;
  }

  jit = std::move(*jitOrErr);
  jitInitialized = true;

  std::cout << "[JIT] Initialized LLVM ORC JIT v2\n";
  return true;
}

llvm::FunctionType* JITCompiler::createFunctionType(const Module::func_type_t& funcType,
                                                     llvm::LLVMContext& ctx) {
  // For PoC: assume i32 parameters and return types
  std::vector<llvm::Type*> paramTypes;
  for (const auto& param : funcType.first) {
    // Map WebAssembly types to LLVM types
    switch (static_cast<ValueTypes>(param)) {
      case ValueTypes::I32:
        paramTypes.push_back(llvm::Type::getInt32Ty(ctx));
        break;
      case ValueTypes::I64:
        paramTypes.push_back(llvm::Type::getInt64Ty(ctx));
        break;
      case ValueTypes::F32:
        paramTypes.push_back(llvm::Type::getFloatTy(ctx));
        break;
      case ValueTypes::F64:
        paramTypes.push_back(llvm::Type::getDoubleTy(ctx));
        break;
      default:
        paramTypes.push_back(llvm::Type::getInt32Ty(ctx));
        break;
    }
  }

  // Return type
  llvm::Type* returnType = llvm::Type::getVoidTy(ctx);
  if (funcType.second.size() > 0) {
    switch (static_cast<ValueTypes>(funcType.second[0])) {
      case ValueTypes::I32:
        returnType = llvm::Type::getInt32Ty(ctx);
        break;
      case ValueTypes::I64:
        returnType = llvm::Type::getInt64Ty(ctx);
        break;
      case ValueTypes::F32:
        returnType = llvm::Type::getFloatTy(ctx);
        break;
      case ValueTypes::F64:
        returnType = llvm::Type::getDoubleTy(ctx);
        break;
      default:
        returnType = llvm::Type::getInt32Ty(ctx);
        break;
    }
  }

  return llvm::FunctionType::get(returnType, paramTypes, false);
}

llvm::Function* JITCompiler::translateToIR(uint32_t funcIdx,
                                           const Runtime::RTFuncDescriptor& descriptor,
                                           shared_module_runtime_t rtIns,
                                           llvm::Module* module) {
  llvm::IRBuilder<> builder(*context);

  // Create function
  std::string funcName = "wasm_func_" + std::to_string(funcIdx);
  llvm::FunctionType* funcType = createFunctionType(*descriptor.funcType, *context);
  llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                                 funcName, module);

  // Create entry basic block
  llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", func);
  builder.SetInsertPoint(entry);

  // Allocate locals (parameters + local variables)
  std::vector<llvm::AllocaInst*> locals;

  // Set up parameters
  size_t paramCount = descriptor.funcType->first.size();
  size_t idx = 0;
  for (auto& arg : func->args()) {
    llvm::AllocaInst* alloca = builder.CreateAlloca(arg.getType(), nullptr, "param_" + std::to_string(idx));
    builder.CreateStore(&arg, alloca);
    locals.push_back(alloca);
    idx++;
  }

  // Allocate additional locals from descriptor.localsDefault
  for (size_t i = paramCount; i < descriptor.localsDefault.size(); ++i) {
    const auto& localVal = descriptor.localsDefault[i];
    llvm::Type* localType = nullptr;

    if (std::holds_alternative<Runtime::rt_i32_t>(localVal)) {
      localType = llvm::Type::getInt32Ty(*context);
    } else if (std::holds_alternative<Runtime::rt_i64_t>(localVal)) {
      localType = llvm::Type::getInt64Ty(*context);
    } else if (std::holds_alternative<Runtime::rt_f32_t>(localVal)) {
      localType = llvm::Type::getFloatTy(*context);
    } else if (std::holds_alternative<Runtime::rt_f64_t>(localVal)) {
      localType = llvm::Type::getDoubleTy(*context);
    } else {
      localType = llvm::Type::getInt32Ty(*context);
    }

    llvm::AllocaInst* alloca = builder.CreateAlloca(localType, nullptr, "local_" + std::to_string(i));
    // Initialize to zero
    builder.CreateStore(llvm::Constant::getNullValue(localType), alloca);
    locals.push_back(alloca);
  }

  // WebAssembly stack simulation (for now, use simple vector)
  std::vector<llvm::Value*> stack;

  // Parse bytecode and translate to LLVM IR
  uint8_t* pc = descriptor.codeEntry;
  bool running = true;

  while (running) {
    OpCodes op = static_cast<OpCodes>(*pc++);

    switch (op) {
      case OpCodes::End:
        running = false;
        break;

      case OpCodes::Return: {
        if (funcType->getReturnType()->isVoidTy()) {
          builder.CreateRetVoid();
        } else if (!stack.empty()) {
          llvm::Value* retVal = stack.back();
          stack.pop_back();
          builder.CreateRet(retVal);
        } else {
          builder.CreateRet(llvm::Constant::getNullValue(funcType->getReturnType()));
        }
        running = false;
        break;
      }

      case OpCodes::LocalGet: {
        uint32_t localIdx = Decoder::decodeVaruint<uint32_t>(pc);
        if (localIdx < locals.size()) {
          llvm::Value* val = builder.CreateLoad(locals[localIdx]->getAllocatedType(), locals[localIdx]);
          stack.push_back(val);
        }
        break;
      }

      case OpCodes::LocalSet: {
        uint32_t localIdx = Decoder::decodeVaruint<uint32_t>(pc);
        if (localIdx < locals.size() && !stack.empty()) {
          llvm::Value* val = stack.back();
          stack.pop_back();
          builder.CreateStore(val, locals[localIdx]);
        }
        break;
      }

      case OpCodes::LocalTee: {
        uint32_t localIdx = Decoder::decodeVaruint<uint32_t>(pc);
        if (localIdx < locals.size() && !stack.empty()) {
          llvm::Value* val = stack.back();  // Keep on stack
          builder.CreateStore(val, locals[localIdx]);
        }
        break;
      }

      case OpCodes::I32Const: {
        int32_t constVal = Decoder::decodeVarint<int32_t>(pc);
        stack.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), constVal, true));
        break;
      }

      case OpCodes::I32Add: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* result = builder.CreateAdd(lhs, rhs);
          stack.push_back(result);
        }
        break;
      }

      case OpCodes::I32Sub: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* result = builder.CreateSub(lhs, rhs);
          stack.push_back(result);
        }
        break;
      }

      case OpCodes::I32Mul: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* result = builder.CreateMul(lhs, rhs);
          stack.push_back(result);
        }
        break;
      }

      case OpCodes::I32LtS: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* cmp = builder.CreateICmpSLT(lhs, rhs);
          llvm::Value* result = builder.CreateZExt(cmp, llvm::Type::getInt32Ty(*context));
          stack.push_back(result);
        }
        break;
      }

      case OpCodes::I32GtS: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* cmp = builder.CreateICmpSGT(lhs, rhs);
          llvm::Value* result = builder.CreateZExt(cmp, llvm::Type::getInt32Ty(*context));
          stack.push_back(result);
        }
        break;
      }

      case OpCodes::I32Eq: {
        if (stack.size() >= 2) {
          llvm::Value* rhs = stack.back(); stack.pop_back();
          llvm::Value* lhs = stack.back(); stack.pop_back();
          llvm::Value* cmp = builder.CreateICmpEQ(lhs, rhs);
          llvm::Value* result = builder.CreateZExt(cmp, llvm::Type::getInt32Ty(*context));
          stack.push_back(result);
        }
        break;
      }

      // For PoC, skip complex control flow (If/Loop/Br) - will just interpret those
      case OpCodes::If:
      case OpCodes::Loop:
      case OpCodes::Block:
      case OpCodes::Br:
      case OpCodes::BrIf:
        // Cannot JIT compile functions with complex control flow yet
        std::cout << "[JIT] Skipping function " << funcIdx << " - contains unsupported control flow\n";
        return nullptr;

      default:
        // Unsupported opcode for JIT - skip this function
        std::cout << "[JIT] Skipping function " << funcIdx << " - unsupported opcode: "
                  << static_cast<int>(op) << "\n";
        return nullptr;
    }
  }

  // If we reached natural end without return, add return
  if (!builder.GetInsertBlock()->getTerminator()) {
    if (funcType->getReturnType()->isVoidTy()) {
      builder.CreateRetVoid();
    } else if (!stack.empty()) {
      llvm::Value* retVal = stack.back();
      builder.CreateRet(retVal);
    } else {
      builder.CreateRet(llvm::Constant::getNullValue(funcType->getReturnType()));
    }
  }

  // Verify function
  std::string errorStr;
  llvm::raw_string_ostream errorStream(errorStr);
  if (llvm::verifyFunction(*func, &errorStream)) {
    std::cerr << "[JIT] Function verification failed: " << errorStr << "\n";
    return nullptr;
  }

  return func;
}

bool JITCompiler::compileFunction(uint32_t funcIdx,
                                 Runtime::RTFuncDescriptor& descriptor,
                                 shared_module_runtime_t rtIns) {
  if (!jitInitialized) {
    initialize();
  }

  std::cout << "[JIT] Compiling function " << funcIdx << " (execution count: "
            << descriptor.executionCount << ")\n";

  // Create a new module for this function
  auto module = std::make_unique<llvm::Module>("wasm_jit_module_" + std::to_string(funcIdx), *context);

  // Translate bytecode to LLVM IR
  llvm::Function* func = translateToIR(funcIdx, descriptor, rtIns, module.get());
  if (!func) {
    std::cout << "[JIT] Failed to translate function " << funcIdx << " to IR\n";
    return false;
  }

  // Print IR for debugging
  std::cout << "[JIT] Generated IR for function " << funcIdx << ":\n";
  func->print(llvm::outs());
  std::cout << "\n";

  // Add module to JIT
  auto tsm = llvm::orc::ThreadSafeModule(std::move(module), std::make_unique<llvm::LLVMContext>());
  auto err = jit->addIRModule(std::move(tsm));
  if (err) {
    llvm::errs() << "[JIT] Failed to add IR module: " << llvm::toString(std::move(err)) << "\n";
    return false;
  }

  // Lookup compiled function
  std::string funcName = "wasm_func_" + std::to_string(funcIdx);
  auto symOrErr = jit->lookup(funcName);
  if (!symOrErr) {
    llvm::errs() << "[JIT] Failed to lookup function: " << llvm::toString(symOrErr.takeError()) << "\n";
    return false;
  }

  // Store compiled function pointer
  descriptor.jitCompiledPtr = reinterpret_cast<void*>(symOrErr->getValue());
  descriptor.isJitCompiled = true;

  std::cout << "[JIT] Successfully compiled function " << funcIdx << " at address "
            << descriptor.jitCompiledPtr << "\n";
  return true;
}

void* JITCompiler::lookupCompiledFunction(const std::string& funcName) {
  if (!jitInitialized) {
    return nullptr;
  }

  auto symOrErr = jit->lookup(funcName);
  if (!symOrErr) {
    llvm::errs() << "[JIT] Lookup failed: " << llvm::toString(symOrErr.takeError()) << "\n";
    return nullptr;
  }

  return reinterpret_cast<void*>(symOrErr->getValue());
}

}  // namespace TWVM
