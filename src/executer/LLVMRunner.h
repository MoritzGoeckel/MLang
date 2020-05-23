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

#include "../exceptions/Exceptions.h"
#include "llvm/ExecutionEngine/Interpreter.h"
//#include "llvm/ExecutionEngine/Jit.h"

class LLVMRunner {
   private:
    llvm::ExecutionEngine* engine;
    llvm::Function* main;
    bool isBroken;

   public:
    enum class Result { InvalidModule, Success };

    LLVMRunner(std::unique_ptr<llvm::Module> module)
        : engine(nullptr), main(nullptr), isBroken(false) {
        isBroken = llvm::verifyModule(*module, &llvm::outs());
        if (isBroken) {
            return;
        }

        main = module->getFunction("main");
        engine = llvm::EngineBuilder(std::move(module)).create();
    }

    ~LLVMRunner() {
        if (engine) delete engine;
    }

    bool getIsBroken() { return isBroken; }

    Result run() {
        if (isBroken) {
            return Result::InvalidModule;
        }

        std::vector<llvm::GenericValue> emptyArgs;
        llvm::GenericValue gv = engine->runFunction(main, emptyArgs);

        // TODO, maybe show other return types
        if (main->getReturnType()->isIntegerTy()) {
            llvm::outs() << "Result: " << gv.IntVal << '\n';
        }

        return Result::Success;
    }
};
