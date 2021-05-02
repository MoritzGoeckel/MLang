#pragma once

#include <iostream>
#include <memory>
#include <sstream>

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
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/Exceptions.h"
#include "TreeWalker.h"

/*
 * TODO
 */
class LLVMEmitter {
   private:
    std::unique_ptr<llvm::Module> module;
    std::shared_ptr<llvm::LLVMContext>
        context;  // TODO: Context needs to outlast the module

    std::map<std::string, std::shared_ptr<AST::Function>> functions;

    std::map<std::string, llvm::Function *> llvmFunctions;

    // Alloca allocates on the stack
    std::vector<std::map<std::string, llvm::AllocaInst *>> stack;

    size_t lastId;

    std::map<std::string, llvm::Function *> fns;

   public:
    LLVMEmitter(
        const std::map<std::string, std::shared_ptr<AST::Function>> &functions,
        std::shared_ptr<llvm::LLVMContext> &context);

    // Why does this not happen automatically?
    ~LLVMEmitter();

    void run();

    void print();

    const std::unique_ptr<llvm::Module> getModule();

   private:
    std::string createUnique(const char *str);
    std::string createUnique(std::string str);

    llvm::Function *instantiateFn(const std::string &name,
                                  std::shared_ptr<AST::Function> ast);

    llvm::Type *convertType(DataType::Primitive type);

    llvm::Type *convertType(const DataType &in);

    std::vector<llvm::Type *> convertType(const std::vector<DataType> &in);

    void process(std::shared_ptr<AST::Node> node, llvm::IRBuilder<> &builder);

    // string builder.CreateGlobalStringPtr("value = %d\n"); // TODO

    llvm::Value *getValue(const std::shared_ptr<AST::Node> &node,
                          llvm::IRBuilder<> &builder);

    void followChildren(std::shared_ptr<AST::Node> &node,
                        llvm::IRBuilder<> &builder);
};
