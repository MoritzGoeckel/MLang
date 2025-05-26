#include "ByteCodeEmitter.h"

#include "../error/Exceptions.h"
#include "../core/Logger.h"


namespace emitter {

ByteCodeEmitter::ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : Emitter(functions), code{}, backpatches{}, localNames{} {}


executor::Program  ByteCodeEmitter::getProgram() {
    return executor::Program{data, code};
}

void ByteCodeEmitter::run() {
    std::map<std::string, size_t> function_idxs;

    code.push_back(executor::Instruction(executor::Op::PUSH, 0));
    code.push_back(executor::Instruction(executor::Op::CALL, 0)); // Call main
    code.push_back(executor::Instruction(executor::Op::TERM)); // We finish when we are back

    for (auto &fn : functions) {
        // TODO: All functions need to have unique names. If that is not a given, 
        // we have to ensure this before with some kind of tree walker

        localNames.clear();
        for(const auto& param : fn.second->getHead()->getParameters()) {
            localNames.push_back(param->getName());
        }
        function_idxs[fn.first] = code.size();
        process(fn.second->getBody());
    }

    code.front().arg1 = function_idxs["main"]; // Set the call to main
    for (const auto &bp : backpatches) {
        if (function_idxs.find(bp.label) == function_idxs.end()) {
            std::cout << "Backpatch label: " << bp.label << std::endl;
            for (const auto& fn : function_idxs) {
                std::cout << "Function: " << fn.first << " at index: " << fn.second << std::endl;
            }
            throwConstraintViolated("Backpatch label not found in function indexes.");
        }
        // Must be a push
        code[bp.instruction_idx].arg1 = function_idxs[bp.label];
    }
}

std::string ByteCodeEmitter::toString() {
    return instructionsToString(code, false);
}

void ByteCodeEmitter::loadIdentifier(const std::shared_ptr<AST::Identifier>& identifier) {

    auto it = std::find(localNames.begin(), localNames.end(), identifier->getName());
    if (it == localNames.end()) {
        throwConstraintViolated("Identifier not found in local names.");
    }
    auto localIdx = std::distance(localNames.begin(), it);
    code.push_back(executor::Instruction(executor::Op::LOCALL, localIdx)); // Bring it on the stack
}

void ByteCodeEmitter::storeLocalInto(const std::shared_ptr<AST::Node>& node){
    switch(node->getType()) {
        case AST::NodeType::Identifier: {
            auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
            auto it = std::find(localNames.begin(), localNames.end(), identifier->getName());
            if (it == localNames.end()) {
                throwConstraintViolated("Identifier not found in local names.");
            }  
            auto localIdx = std::distance(localNames.begin(), it);
            code.push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            break;
        }
        case AST::NodeType::Declvar: {
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            auto localIdx = localNames.size();
            code.push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            localNames.push_back(declvar->getIdentifier()->getName());
            break;
        }
        default:{
            std::cout << node->toString() << std::endl;
            throwConstraintViolated("Invalid LValue type.");
        }
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
            } 
            code.push_back(executor::Instruction(executor::Op::RET));
            break;
        }
        case AST::NodeType::Assign: {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            process(assign->getRight());
            storeLocalInto(assign->getLeft());
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
                loadIdentifier(call->getIdentifier()); // Bring the function addr on the stack
                code.push_back(executor::Instruction(executor::Op::CALL, call->getArguments().size()));
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
        case AST::NodeType::FnPtr: {
            auto fnPtr = std::dynamic_pointer_cast<AST::FnPtr>(node);
            // Later backpatch the address
            backpatches.push_back(Backpatch{code.size(), fnPtr->getId()});
            code.push_back(executor::Instruction(executor::Op::PUSH, 0));
            break;
        }
        case AST::NodeType::Identifier: {
            loadIdentifier(std::dynamic_pointer_cast<AST::Identifier>(node));
            break;
        }
        case AST::NodeType::Declvar: {
            // Declare new variable with inital value 0
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            auto localIdx = localNames.size();
            code.push_back(executor::Instruction(executor::Op::PUSH, 0)); // Initial value
            code.push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            localNames.push_back(declvar->getIdentifier()->getName());
            break;
        }
    }
}

} // namespace emitter