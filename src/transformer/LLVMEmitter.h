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
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TODO
 */
class LLVMEmitter : TreeWalker {
   private:
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::LLVMContext> context;

    std::map<std::string, std::shared_ptr<AST::Function>> functions;

   public:
    LLVMEmitter(
        const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
        : context(), module(), functions(functions) {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("test", *context);
    }

    // Why does this not happen automatically?
    ~LLVMEmitter() { module.reset(); }

    void run() {}

   private:
    // void instantiateFunction
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        // Block
        if (node->getType() == AST::NodeType::Block) {
            // followChildren(node);
        }

        // Declfn
        else if (node->getType() == AST::NodeType::Declfn) {
            // auto declfn = std::dynamic_pointer_cast<AST::Declfn>(node);
        }

        // Any other node
        else {
            followChildren(node);
        }

        return node;
    }

    void test() {
        // Create the add1 function entry and insert this entry into module M.
        // The function will have a return type of "int" and take an argument of
        // "int".
        llvm::Function *Add1F = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*context),
                                    {llvm::Type::getInt32Ty(*context)}, false),
            llvm::Function::ExternalLinkage, "add1", *module);

        // Add a basic block to the function. As before, it automatically
        // inserts because of the last argument.
        llvm::BasicBlock *BB =
            llvm::BasicBlock::Create(*context, "EntryBlock", Add1F);

        // Create a basic block builder with default parameters.  The builder
        // will automatically append instructions to the basic block `BB'.
        llvm::IRBuilder<> builder(BB);

        // Get pointers to the constant `1'.
        llvm::Value *One = builder.getInt32(1);

        // Get pointers to the integer argument of the add1 function...
        assert(Add1F->arg_begin() !=
               Add1F->arg_end());  // Make sure there's an arg
        llvm::Argument *ArgX = &*Add1F->arg_begin();  // Get the arg
        ArgX->setName("AnArg");  // Give it a nice symbolic name for fun.

        // Create the add instruction, inserting it into the end of BB.
        llvm::Value *Add = builder.CreateAdd(One, ArgX);

        // Create the return instruction and add it to the basic block
        builder.CreateRet(Add);

        // Now, function add1 is ready.

        // Now we're going to create function `foo', which returns an int and
        // takes no arguments.
        llvm::Function *FooF = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), {},
                                    false),
            llvm::Function::ExternalLinkage, "foo", *module);

        // Add a basic block to the FooF function.
        BB = llvm::BasicBlock::Create(*context, "EntryBlock", FooF);

        // Tell the basic block builder to attach itself to the new basic block
        builder.SetInsertPoint(BB);

        // Get pointer to the constant `10'.
        llvm::Value *Ten = builder.getInt32(10);

        // Pass Ten to the call to Add1F
        llvm::CallInst *Add1CallRes = builder.CreateCall(Add1F, Ten);
        Add1CallRes->setTailCall(true);

        // Create the return instruction and add it to the basic block.
        builder.CreateRet(Add1CallRes);
    }
};
