#pragma once

#include <iostream>
#include <memory>

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "../Mlang.h"
#include "../error/Exceptions.h"

#include "llvm/ExecutionEngine/Interpreter.h"
//#include "llvm/ExecutionEngine/Jit.h"

class LLVMRunner {
   private:
    llvm::ExecutionEngine* engine;
    std::shared_ptr<llvm::LLVMContext> context;
    llvm::Function* main;
    bool isBroken;

   public:
    // TODO: Maybe make it a move constructor?
    LLVMRunner(std::unique_ptr<llvm::Module> module,
               std::shared_ptr<llvm::LLVMContext>& context);
    ~LLVMRunner();

    bool addModule(const std::string& code);
    bool addModule(std::unique_ptr<llvm::Module>&& module);

    bool getIsBroken();
    Mlang::Result run();
};
