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

   public:
    LLVMRunner(std::unique_ptr<llvm::Module> module) {
        llvm::verifyModule(*module);  // Seems to not work? TODO
        main = module->getFunction("main");

        engine = llvm::EngineBuilder(std::move(module)).create();  // Move?

        if (!main) {
            std::cout << "main is nullptr" << std::endl;
        }

        if (!engine) {
            std::cout << "engine is nullptr" << std::endl;
        }

        // TODO, will run into segfaults if the function does not end with an
        // return. Need implicit returns when handling void functions

        std::vector<llvm::GenericValue> emptyArgs;
        llvm::GenericValue gv = engine->runFunction(main, emptyArgs);

        // llvm::outs() << *module;
        // TODO, maybe show other return types
        if (main->getReturnType()->isIntegerTy()) {
            llvm::outs() << "Result: " << gv.IntVal << '\n';
        }
    }

    ~LLVMRunner() { delete engine; }

    void run() {
        /*std::cout << "3" << std::endl;
        std::vector<llvm::GenericValue> emptyArgs;
        engine->runFunction(main, emptyArgs);  // llvm::GenericValue gv =
        std::cout << "4" << std::endl;*/
        // llvm::outs() << "Result: " << gv.IntVal << "\n"; // TODO
    }
};
