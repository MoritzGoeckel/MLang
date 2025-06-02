#include "ByteCodeEmitter.h"

#include "../error/Exceptions.h"
#include "../core/Logger.h"


namespace emitter {

ByteCodeEmitter::ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : functions(functions), code{}, backpatches{}, localNames{} {}


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
        process(fn.second->getBody(), false);
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

void ByteCodeEmitter::process(const std::shared_ptr<AST::Node>& node, bool hasConsumer) {
    switch(node->getType()) {
        case AST::NodeType::Block: {
            for(const auto& child : std::dynamic_pointer_cast<AST::Block>(node)->getChildren()) {
                process(child, false);
            }
            break;
        }
        case AST::NodeType::Ret: {
            auto ret = std::dynamic_pointer_cast<AST::Ret>(node);
            if (ret->getExpr()) {
                process(ret->getExpr(), true);
            }
            code.push_back(executor::Instruction(executor::Op::RET));
            break;
        }
        case AST::NodeType::Assign: {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            process(assign->getRight(), true);
            storeLocalInto(assign->getLeft());
            break;
        }
        case AST::NodeType::If: {
            auto ifNode = std::dynamic_pointer_cast<AST::If>(node);
            process(ifNode->getCondition(), true);
            auto jumpIfIdx = code.size();
            code.push_back(executor::Instruction(executor::Op::JUMP_IF, 0)); // Go to else or end
            process(ifNode->getPositive(), false);

            if (ifNode->getNegative()) {
                auto jumpEndIdx = code.size();
                code.push_back(executor::Instruction(executor::Op::JUMP, 0)); // Go to end
                process(ifNode->getNegative(), false);
                auto endIdx = code.size();
                code.push_back(executor::Instruction(executor::Op::NOP));
                code[jumpIfIdx].arg1 = jumpEndIdx + 1; // Backpatch the jump if, skip to end
                code[jumpEndIdx].arg1 = endIdx; // Backpatch the jump to end
            } else {
                auto endIdx = code.size();
                code.push_back(executor::Instruction(executor::Op::NOP));
                code[jumpIfIdx].arg1 = endIdx; // Backpatch the jump if, skip to end
            }
            break;
        }
        case AST::NodeType::While: {
            auto whileNode = std::dynamic_pointer_cast<AST::While>(node);
            auto startIdx = code.size();
            process(whileNode->getCondition(), true);
            auto jumpIfIdx = code.size();
            code.push_back(executor::Instruction(executor::Op::JUMP_IF, 0)); // Go to end if false
            process(whileNode->getBody(), false);
            code.push_back(executor::Instruction(executor::Op::JUMP, startIdx)); // Jump back to condition
            auto endIdx = code.size();
            code.push_back(executor::Instruction(executor::Op::NOP));
            code[jumpIfIdx].arg1 = endIdx; // Backpatch the jump if, skip to end
            break;
        }
        case AST::NodeType::Call: {
            auto call = std::dynamic_pointer_cast<AST::Call>(node);

            for(const auto& arg : call->getArguments()) {
               process(arg, true);
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
            } else if (fnName == "<") {
                code.push_back(executor::Instruction(executor::Op::LT));
            } else if (fnName == ">") {
                code.push_back(executor::Instruction(executor::Op::GT));
            } else if (fnName == "==") {
                code.push_back(executor::Instruction(executor::Op::EQ));
            } else if (fnName == "<=") {
                code.push_back(executor::Instruction(executor::Op::LTE));
            } else if (fnName == ">=") {
                code.push_back(executor::Instruction(executor::Op::GTE));
            } else if (fnName == "!=") {
                code.push_back(executor::Instruction(executor::Op::NEQ));
            } else {
                loadIdentifier(call->getIdentifier()); // Bring the function addr on the stack
                code.push_back(executor::Instruction(executor::Op::CALL, call->getArguments().size()));

                // CALL consumes the arguments from the stack, so we don't need to pop them
                // If we don't have a consumer, we have to pop the result. We assume only one result.
                // Only if the function is not void, of course.

                const auto& identifier = call->getIdentifier();
                ASSURE_NOT_NULL(identifier);
                const auto& returnType = identifier->getDataType().getReturn();
                ASSURE_NOT_NULL(returnType);
                if(!hasConsumer && *returnType != DataType::Primitive::None) {
                    code.push_back(executor::Instruction(executor::Op::POP));
                }
            }

            break;
        }
        case AST::NodeType::Literal: {
            if(hasConsumer) {
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
            }
            break;
        }
        case AST::NodeType::FnPtr: {
            if(hasConsumer) {
                auto fnPtr = std::dynamic_pointer_cast<AST::FnPtr>(node);
                // Later backpatch the address
                backpatches.push_back(Backpatch{code.size(), fnPtr->getId()});
                code.push_back(executor::Instruction(executor::Op::PUSH, 0));
            }
            break;
        }
        case AST::NodeType::Identifier: {
            if (hasConsumer) {
                loadIdentifier(std::dynamic_pointer_cast<AST::Identifier>(node));
            }
            break;
        }
        case AST::NodeType::Declvar: {
            // Declare new variable with inital value 0
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            auto localIdx = localNames.size();
            localNames.push_back(declvar->getIdentifier()->getName());

            const auto& dataType = declvar->getIdentifier()->getDataType();

            if(dataType.isPrimitive()) {
                code.push_back(executor::Instruction(executor::Op::PUSH, 0)); // Initial value
                code.push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            } else if(dataType.isStruct()) {
                // structTypes.
                const auto& structType = dataType.getStruct();
                std::cout << "Name=" << structType.name << std::endl;
                for (const auto& [fieldName, fieldType] : structType.fields) {
                    std::cout << "Field: " << fieldType.toString() << std::endl;
                }
                // Need to look them up in  structTypes by name
                throwTodo("Structs not yet supported in variable declaration");
                // TODO
            }
            break;
        }
        case AST::NodeType::DeclStruct: {
            // Are collected in TypesCollector
            break;
        }
    }
}

} // namespace emitter
