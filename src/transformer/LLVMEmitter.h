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
#include "../exceptions/Exceptions.h"
#include "TreeWalker.h"

/*
 * TODO
 */
class LLVMEmitter {
   private:
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::LLVMContext> context;

    std::map<std::string, std::shared_ptr<AST::Function>> functions;

    std::map<std::string, llvm::Function *> llvmFunctions;

    // Alloca allaocates on the stack
    std::vector<std::map<std::string, llvm::AllocaInst *>> stack;

   public:
    LLVMEmitter(
        const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
        : context(), module(), functions(functions) {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("test", *context);
    }

    // Why does this not happen automatically?
    ~LLVMEmitter() { module.reset(); }

    void run() {
        std::map<std::string, llvm::Function *> fns;
        for (auto &fn : functions) {
            fns[fn.first] = instantiateFn(fn.first, fn.second);
        }
    }

    void print() {
        llvm::outs() << "Module:\n\n" << *module;
        llvm::outs().flush();
    }

    const std::unique_ptr<llvm::Module> getModule() {
        return std::move(module);
    }

   private:
    llvm::Function *instantiateFn(const std::string &name,
                                  std::shared_ptr<AST::Function> ast) {
        const auto &type = ast->getHead()->getIdentifier()->getDataType();

        auto fn = llvm::Function::Create(
            llvm::FunctionType::get(convertType(*type.getReturn()),
                                    convertType(*type.getParams()),
                                    false /*isVararg*/),
            llvm::Function::ExternalLinkage, name, *module);

        llvm::BasicBlock *block =
            llvm::BasicBlock::Create(*context, name + "Block", fn);

        llvm::IRBuilder<> builder(block);

        stack.push_back({});
        // Push params to stack
        const auto &params = ast->getHead()->getParameters();
        size_t i = 0;
        for (auto &param : fn->args()) {
            auto name = params[i]->getName();

            // Declare
            auto type = convertType(params[i]->getDataType());
            if (!params[i]->getDataType().getIsPrimitive()) {
                // Need to convert function types to pointers for the alloca
                // but not for anything else. In other places (call, store
                // and load) we still need the non-pointer type
                type = type->getPointerTo();
            }

            auto *alloc = builder.CreateAlloca(
                convertType(params[i]->getDataType()), nullptr, name);

            // Store value
            builder.CreateStore(&param, alloc, /*isVolatile=*/false);

            // Put on stack
            stack.back().emplace(name, alloc);
            i++;
        }
        process(ast->getBody(), builder);
        stack.pop_back();

        if (type.getReturn()->getIsPrimitive() &&
            type.getReturn()->getPrimitive() == DataType::Primitive::None) {
            // TODO Should be void?
            builder.CreateRetVoid();
        }

        llvmFunctions[name] = fn;
        return fn;  // Fishy... why ptr? What about shared_ptr?
    }

    llvm::Type *convertType(DataType::Primitive type) {
        switch (type) {
            case DataType::Primitive::Int:
                return llvm::Type::getInt32Ty(*context);
            case DataType::Primitive::Float:
                return llvm::Type::getFloatTy(*context);
            case DataType::Primitive::String:
                throwTodo("String not yet supported");
            case DataType::Primitive::Bool:
                return llvm::Type::getInt1Ty(*context);

            case DataType::Primitive::Void:
            case DataType::Primitive::None:
                return llvm::Type::getVoidTy(*context);

            case DataType::Primitive::Conflict:
                throwConstraintViolated("Conflict in types");
            case DataType::Primitive::Unknown:
                throwConstraintViolated("Unknown in types");
            default:
                throwConstraintViolated("Type not caught");
        }
    }

    llvm::Type *convertType(const DataType &in) {
        if (in.getIsPrimitive()) {
            return convertType(in.getPrimitive());
        } else {
            return llvm::FunctionType::get(convertType(*in.getReturn()),
                                           convertType(*in.getParams()), false);
        }
    }

    std::vector<llvm::Type *> convertType(const std::vector<DataType> &in) {
        std::vector<llvm::Type *> out;
        out.reserve(in.size());
        for (const auto &t : in) {
            out.push_back(convertType(t));
        }
        return out;
    }

    void process(std::shared_ptr<AST::Node> node, llvm::IRBuilder<> &builder) {
        if (node->getType() == AST::NodeType::Block) {
            stack.push_back({});
            followChildren(node, builder);
            stack.pop_back();

        } else if (node->getType() == AST::NodeType::Ret) {
            auto ptr = std::dynamic_pointer_cast<AST::Ret>(node);
            auto child = ptr->getExpr();
            if (child)
                builder.CreateRet(getValue(child, builder));
            else
                builder.CreateRetVoid();

        } else if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);

            if (assign->getLeft()->getType() == AST::NodeType::Declfn) {
                throwConstraintViolated("Declfn should be gone by now");
            }

            // Right side
            auto right = assign->getRight();
            auto *value = getValue(right, builder);

            // Left side
            std::string name;
            if (assign->getLeft()->getType() == AST::NodeType::Declvar) {
                // Declaration
                auto declvar =
                    std::dynamic_pointer_cast<AST::Declvar>(assign->getLeft());
                name = declvar->getIdentifier()->getName();

                auto type = declvar->getIdentifier()->getDataType();
                auto llvmType = convertType(type);
                if (!type.getIsPrimitive()) {
                    // Need to convert function types to pointers for the alloca
                    // but not for anything else. In other places (call, store
                    // and load) we still need the non-pointer type
                    llvmType = llvmType->getPointerTo();
                }
                auto *alloc = builder.CreateAlloca(llvmType, nullptr, name);

                // Push to stack
                if (stack.back().find(name) != stack.back().end()) {
                    throwConstraintViolated("Variable already declared");
                }
                stack.back().emplace(name, alloc);
            } else {
                name = std::dynamic_pointer_cast<AST::Identifier>(
                           assign->getLeft())
                           ->getName();
            }

            // Get from stack
            llvm::AllocaInst *alloc = nullptr;
            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                if (rIt->find(name) != rIt->end()) {
                    alloc = (*rIt)[name];
                    break;
                }
            }

            if (!alloc) throwConstraintViolated("Varaible not found on stack");
            builder.CreateStore(value, alloc, /*isVolatile=*/false);

        } else if (node->getType() == AST::NodeType::If) {
            auto ifNode = std::dynamic_pointer_cast<AST::If>(node);

            // llvm::IRBuilder<> builder(block);
            llvm::Function *fn = builder.GetInsertBlock()->getParent();
            auto hostBlock = builder.GetInsertBlock();

            // Create trueBlock
            auto *thenBlock = llvm::BasicBlock::Create(*context, "thenBlock");
            auto *elseBlock = llvm::BasicBlock::Create(*context, "elseBlock");
            auto *mergeBlock = llvm::BasicBlock::Create(*context, "mergeBlock");
            bool needMergeBlock = false;

            builder.SetInsertPoint(thenBlock);
            process(ifNode->getPositive(), builder);
            if (!thenBlock->getTerminator()) {
                // Emit branch if no return exists
                builder.CreateBr(mergeBlock);
                needMergeBlock = true;
            }

            // TODO: Could optimize some branches if no else exists
            builder.SetInsertPoint(elseBlock);
            if (ifNode->getNegative()) {
                process(ifNode->getNegative(), builder);
            }
            if (!elseBlock->getTerminator()) {
                // Emit branch if no return exists
                builder.CreateBr(mergeBlock);
                needMergeBlock = true;
            }

            builder.SetInsertPoint(hostBlock);
            llvm::Value *condition = getValue(ifNode->getCondition(), builder);
            builder.CreateCondBr(condition, thenBlock, elseBlock);

            fn->getBasicBlockList().push_back(thenBlock);
            fn->getBasicBlockList().push_back(elseBlock);
            if (needMergeBlock) {
                fn->getBasicBlockList().push_back(mergeBlock);
                builder.SetInsertPoint(mergeBlock);  // Cont in the last block
                // TODO wierd stuff happens if there comes more unreachable code
            }

        } else if (node->getType() == AST::NodeType::While) {
            throwTodo("While not implemented yet");  // TODO while
        } else {
            // followChildren(node, builder);
            throwTodo("Unexpected / not implemented element");
        }
    }

    // string builder.CreateGlobalStringPtr("value = %d\n"); // TODO

    llvm::Value *getValue(const std::shared_ptr<AST::Node> &node,
                          llvm::IRBuilder<> &builder) {
        if (node->getType() == AST::NodeType::Call) {
            auto call = std::dynamic_pointer_cast<AST::Call>(node);
            const auto &name = call->getIdentifier()->getName();
            const auto &type = call->getIdentifier()->getDataType();

            std::vector<llvm::Value *> args;
            args.reserve(call->getArguments().size());
            for (auto &arg : call->getArguments()) {
                args.emplace_back(getValue(arg, builder));
            }

            if (args.size() == 2u) {
                // Buildin functions
                // TODO: Check types, FAdd / FMul / Neg
                // TODO: Less / Equal / etc
                if (name == "+") {
                    return builder.CreateAdd(args[0], args[1], "addtmp");
                } else if (name == "-") {
                    return builder.CreateSub(args[0], args[1], "subtmp");
                } else if (name == "*") {
                    return builder.CreateMul(args[0], args[1], "multmp");
                } else if (name == "/") {
                    return builder.CreateSDiv(args[0], args[1], "sdivtmp");
                } else if (name == "&&") {
                    return builder.CreateAnd(args[0], args[1], "andtmp");
                } else if (name == "||") {
                    return builder.CreateOr(args[0], args[1], "ortmp");
                } else if (name == "<") {
                    return builder.CreateICmpSLT(args[0], args[1], "lttmp");
                } else if (name == ">") {
                    return builder.CreateICmpSGT(args[0], args[1], "gttmp");
                }
            }

            // It's not a buildin, lets generate a call to a custom function

            // Load the function pointer
            llvm::AllocaInst *alloc = nullptr;
            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                if (rIt->find(name) != rIt->end()) {
                    alloc = (*rIt)[name];
                    break;
                }
            }

            // TODO: Ugly
            if (!alloc)
                throwConstraintViolated(
                    std::string(std::string("Varaible not found on stack: ") +
                                name)
                        .c_str());

            auto value = builder.CreateLoad(alloc);

            // Create call
            return builder.CreateCall(
                static_cast<llvm::FunctionType *>(convertType(type)), value,
                args);

        } else if (node->getType() == AST::NodeType::Identifier) {
            // Get from stack
            auto name =
                std::dynamic_pointer_cast<AST::Identifier>(node)->getName();
            llvm::AllocaInst *alloc = nullptr;
            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                if (rIt->find(name) != rIt->end()) {
                    alloc = (*rIt)[name];
                    break;
                }
            }

            if (!alloc) throwConstraintViolated("Varaible not found on stack");
            return builder.CreateLoad(alloc);

        } else if (node->getType() == AST::NodeType::Literal) {
            auto literal = std::dynamic_pointer_cast<AST::Literal>(node);
            const auto &type = literal->getDataType();

            if (!type.getIsPrimitive())
                throwTodo("Only primitive types supported");
            switch (type.getPrimitive()) {
                case DataType::Primitive::Int:
                    return builder.getInt32(literal->getIntValue());
                case DataType::Primitive::Float:
                    return builder.Insert(
                        llvm::ConstantFP::get(llvm::Type::getFloatTy(*context),
                                              literal->getFloatValue()),
                        "name");
                case DataType::Primitive::String:
                    throwTodo("String not yet supported");
                case DataType::Primitive::Bool:
                    return builder.getInt1(literal->getBoolValue());

                case DataType::Primitive::Void:
                case DataType::Primitive::None:
                case DataType::Primitive::Conflict:
                case DataType::Primitive::Unknown:
                default:
                    throwConstraintViolated("Should not happen");
            }

        } else if (node->getType() == AST::NodeType::FnPtr) {
            // Function pointers are like literals, they are constant
            auto fnptr = std::dynamic_pointer_cast<AST::FnPtr>(node);
            auto id = fnptr->getId();
            if (llvmFunctions.find(id) == llvmFunctions.end())
                throwConstraintViolated("Function not found for fnptr");
            return llvmFunctions[id];

        } else {
            throwConstraintViolated("Getting value of something unexpected");
        }
    }

    void followChildren(std::shared_ptr<AST::Node> &node,
                        llvm::IRBuilder<> &builder) {
        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child, builder);
        }
    }

    void test() {
        // Create the add1 function entry and insert this entry into module
        // M. The function will have a return type of "int" and take an
        // argument of "int".
        llvm::Function *Add1F = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*context),
                                    {llvm::Type::getInt32Ty(*context)}, false),
            llvm::Function::ExternalLinkage, "add1", *module);

        // Add a basic block to the function. As before, it automatically
        // inserts because of the last argument.
        llvm::BasicBlock *BB =
            llvm::BasicBlock::Create(*context, "EntryBlock", Add1F);

        // Create a basic block builder with default parameters.  The
        // builder will automatically append instructions to the basic block
        // `BB'.
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

        // Now we're going to create function `foo', which returns an int
        // and takes no arguments.
        llvm::Function *FooF = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), {},
                                    false),
            llvm::Function::ExternalLinkage, "foo", *module);

        // Add a basic block to the FooF function.
        BB = llvm::BasicBlock::Create(*context, "EntryBlock", FooF);

        // Tell the basic block builder to attach itself to the new basic
        // block
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
