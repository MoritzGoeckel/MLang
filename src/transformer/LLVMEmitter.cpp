#include "LLVMEmitter.h"

LLVMEmitter::LLVMEmitter(
    const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : context(), module(), functions(functions), lastId(0u) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("test", *context);
}

// Why does this not happen automatically?
LLVMEmitter::~LLVMEmitter() { module.reset(); }

void LLVMEmitter::run() {
    std::map<std::string, llvm::Function *> fns;
    for (auto &fn : functions) {
        fns[fn.first] = instantiateFn(fn.first, fn.second);
    }
}

void LLVMEmitter::print() {
    llvm::outs() << "Module:\n\n" << *module;
    llvm::outs().flush();
}

const std::unique_ptr<llvm::Module> LLVMEmitter::getModule() {
    return std::move(module);
}

std::string LLVMEmitter::createUnique(const char *str) {
    return std::string(str) + "_" + std::to_string(lastId++);
}
std::string LLVMEmitter::createUnique(std::string str) {
    return createUnique(str.c_str());
}

llvm::Function *LLVMEmitter::instantiateFn(const std::string &name,
                                           std::shared_ptr<AST::Function> ast) {
    const auto &type = ast->getHead()->getIdentifier()->getDataType();

    auto fn = llvm::Function::Create(
        llvm::FunctionType::get(convertType(*type.getReturn()),
                                convertType(*type.getParams()),
                                false /*isVararg*/),
        llvm::Function::ExternalLinkage, name, module.get());

    llvm::BasicBlock *block =
        llvm::BasicBlock::Create(*context, createUnique(name), fn);

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

llvm::Type *LLVMEmitter::convertType(DataType::Primitive type) {
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

llvm::Type *LLVMEmitter::convertType(const DataType &in) {
    if (in.getIsPrimitive()) {
        return convertType(in.getPrimitive());
    } else {
        return llvm::FunctionType::get(convertType(*in.getReturn()),
                                       convertType(*in.getParams()), false);
    }
}

std::vector<llvm::Type *> LLVMEmitter::convertType(
    const std::vector<DataType> &in) {
    std::vector<llvm::Type *> out;
    out.reserve(in.size());
    for (const auto &t : in) {
        out.push_back(convertType(t));
    }
    return out;
}

void LLVMEmitter::process(std::shared_ptr<AST::Node> node,
                          llvm::IRBuilder<> &builder) {
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
            name = std::dynamic_pointer_cast<AST::Identifier>(assign->getLeft())
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
        auto *thenBlock =
            llvm::BasicBlock::Create(*context, createUnique("then"), fn);
        auto *elseBlock =
            llvm::BasicBlock::Create(*context, createUnique("else"), fn);
        llvm::BasicBlock *mergeBlock = nullptr;

        builder.SetInsertPoint(thenBlock);
        process(ifNode->getPositive(), builder);
        if (!thenBlock->getTerminator()) {
            // Emit branch if no return exists
            mergeBlock =
                llvm::BasicBlock::Create(*context, createUnique("merge"), fn);
            builder.CreateBr(mergeBlock);
        }

        // TODO: Could optimize some branches if no else exists
        builder.SetInsertPoint(elseBlock);
        if (ifNode->getNegative()) {
            process(ifNode->getNegative(), builder);
        }
        if (!elseBlock->getTerminator()) {
            // Emit branch if no return exists
            if (!mergeBlock) {
                mergeBlock = llvm::BasicBlock::Create(
                    *context, createUnique("merge"), fn);
            }
            builder.CreateBr(mergeBlock);
        }

        builder.SetInsertPoint(hostBlock);
        llvm::Value *condition = getValue(ifNode->getCondition(), builder);
        builder.CreateCondBr(condition, thenBlock, elseBlock);

        if (mergeBlock) {
            builder.SetInsertPoint(mergeBlock);
        } else {
            // TODO: No more code should follow here. It would be
            // unreachable. Assert that.
        }

    } else if (node->getType() == AST::NodeType::While) {
        auto whileNode = std::dynamic_pointer_cast<AST::While>(node);

        llvm::Function *fn = builder.GetInsertBlock()->getParent();

        auto *conditionBlock =
            llvm::BasicBlock::Create(*context, createUnique("condition"), fn);
        auto *bodyBlock =
            llvm::BasicBlock::Create(*context, createUnique("body"), fn);
        auto *afterBlock =
            llvm::BasicBlock::Create(*context, createUnique("after"), fn);

        builder.CreateBr(conditionBlock);
        builder.SetInsertPoint(conditionBlock);

        llvm::Value *condition = getValue(whileNode->getCondition(), builder);
        builder.CreateCondBr(condition, bodyBlock, afterBlock);

        builder.SetInsertPoint(bodyBlock);
        process(whileNode->getBody(), builder);
        builder.CreateBr(conditionBlock);

        builder.SetInsertPoint(afterBlock);

    } else {
        // followChildren(node, builder);
        throwTodo("Unexpected / not implemented element");
    }
}

// string builder.CreateGlobalStringPtr("value = %d\n"); // TODO

llvm::Value *LLVMEmitter::getValue(const std::shared_ptr<AST::Node> &node,
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
                std::string(std::string("Varaible not found on stack: ") + name)
                    .c_str());

        auto value = builder.CreateLoad(alloc);

        // Create call
        return builder.CreateCall(
            static_cast<llvm::FunctionType *>(convertType(type)), value, args);

    } else if (node->getType() == AST::NodeType::Identifier) {
        // Get from stack
        auto name = std::dynamic_pointer_cast<AST::Identifier>(node)->getName();
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

        if (!type.getIsPrimitive()) throwTodo("Only primitive types supported");
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

void LLVMEmitter::followChildren(std::shared_ptr<AST::Node> &node,
                                 llvm::IRBuilder<> &builder) {
    for (auto child : node->getChildren()) {
        if (!child) continue;
        process(child, builder);
    }
}
