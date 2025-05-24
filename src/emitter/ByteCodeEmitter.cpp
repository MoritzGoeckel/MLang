#include "ByteCodeEmitter.h"

#include "../Logger.h"


namespace emitter {

ByteCodeEmitter::ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : Emitter(functions), code{}, labels{} {}

void ByteCodeEmitter::run() {
    for (auto &fn : functions) {
        instantiateFn(fn.first, fn.second);
    }
}

std::string ByteCodeEmitter::toString() {
    return instructionsToString(code, false);
}

executor::word_t determineStackSize(const std::shared_ptr<AST::Function>& ast) {
    // Every parameter
    // Every local variable
    return 100; // TODO
}

void ByteCodeEmitter::instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast) {
    labels[name] = code.size(); 
    process(ast->getBody());
    if(name == "main") {
        code.push_back(executor::Instruction(executor::Op::TERM));
    }
}

void ByteCodeEmitter::process(const std::shared_ptr<AST::Node>& node) {
    switch(node->getType()) {
        case AST::NodeType::Block: {
            followChildren(node);
            break;
        }
        case AST::NodeType::Ret: {
            auto ret = std::dynamic_pointer_cast<AST::Ret>(node);
            if (ret->getExpr()) {
                process(ret->getExpr());
            } else {
                // Write 0 to the destination stack address, we return None
                code.push_back(executor::Instruction(executor::Op::PUSH, 0));
            }
            break;
        }
        case AST::NodeType::Assign: {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            // process_inline(assign->getLeft());
            // code << " = ";
            // process_inline(assign->getRight());
            // code << "\n";
            break;
        }
        case AST::NodeType::If: {
            auto ifNode = std::dynamic_pointer_cast<AST::If>(node);
            // code << "if ";
            // process_inline(ifNode->getCondition());
            // code << ":\n";
            followChildren(ifNode->getPositive());
            if (ifNode->getNegative()) {
                // code << "else:\n";
                followChildren(ifNode->getNegative());
            }
            break;
        }
        case AST::NodeType::Call: {
            auto call = std::dynamic_pointer_cast<AST::Call>(node);

            for(const auto& arg : call->getArguments()) {
               process(arg);
            }

            const auto& fnName = call->getIdentifier()->getName();
            if(fnName == "+"){
                code.push_back(executor::Instruction(executor::Op::ADD));
            } else if (fnName == "-") {
                code.push_back(executor::Instruction(executor::Op::SUB));
            } else if (fnName == "*") {
                code.push_back(executor::Instruction(executor::Op::MUL));
            } else if (fnName == "/") {
                code.push_back(executor::Instruction(executor::Op::DIV));
            } else if (fnName == "%") {
                code.push_back(executor::Instruction(executor::Op::MOD));
            } else {
                // TODO handle function calls
            }

            break;
        }
        case AST::NodeType::Literal: {
            auto literal = std::dynamic_pointer_cast<AST::Literal>(node);
            if (literal->getDataType() == DataType::Primitive::String) {
                // code << "'" << literal->getStringValue() << "'";
            } else if (literal->getDataType() == DataType::Primitive::Bool) {
                code.push_back(executor::Instruction(
                    executor::Op::PUSH, literal->getBoolValue() ? 1 : 0));
            }
            else if (literal->getDataType() == DataType::Primitive::Int) {
                code.push_back(executor::Instruction(
                    executor::Op::PUSH, literal->getIntValue()));
            } else if (literal->getDataType() == DataType::Primitive::Float) {
                code.push_back(executor::Instruction(
                    executor::Op::PUSH, literal->getFloatValue()));
            } 
            break;
        }
        case AST::NodeType::Identifier: {
            auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
            // code << formatName(identifier->getName());
            break;
        }
        case AST::NodeType::Declvar: {
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            // code << formatName(declvar->getIdentifier()->getName());
            break;
        }
        case AST::NodeType::FnPtr: {
            auto fnPtr = std::dynamic_pointer_cast<AST::FnPtr>(node);
            // code << formatName(fnPtr->getId()); // TODO: Rename to getName
            break;
        }
    }
}

} // namespace emitter