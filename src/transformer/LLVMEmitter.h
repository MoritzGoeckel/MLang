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
    llvm::LLVMContext context;

   public:
    LLVMEmitter()
        : context(), module(std::make_unique<llvm::Module>("test", context)) {}

    void run() {}

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
};
