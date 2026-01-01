#include "ByteCodeEmitter.h"

#include "../error/Exceptions.h"
#include "../core/Logger.h"


namespace emitter {

ByteCodeEmitter::ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : functions(functions), program{}, backpatches{}, localNames{} {}


executor::Program  ByteCodeEmitter::getProgram() {
    return executor::Program{program.data, program.code};
}

void ByteCodeEmitter::run() {
    std::map<std::string, size_t> function_idxs;

    code().push_back(executor::Instruction(executor::Op::PUSH, 0));
    code().push_back(executor::Instruction(executor::Op::CALL, 0)); // Call main
    code().push_back(executor::Instruction(executor::Op::TERM)); // We finish when we are back

    for (auto &fn : functions) {
        // TODO: All functions need to have unique names. If that is not a given,
        // we have to ensure this before with some kind of tree walker

        localNames.clear();
        for(const auto& param : fn.second->getHead()->getParameters()) {
            localNames.push_back(param->getName());
        }
        function_idxs[fn.first] = code().size();
        process(fn.second->getBody(), false);
    }

    code().front().arg1 = function_idxs["main"]; // Set the call to main
    for (const auto &bp : backpatches) {
        if (function_idxs.find(bp.label) == function_idxs.end()) {
            std::cout << "Backpatch label: " << bp.label << std::endl;
            for (const auto& fn : function_idxs) {
                std::cout << "Function: " << fn.first << " at index: " << fn.second << std::endl;
            }
            throwConstraintViolated("Backpatch label not found in function indexes.");
        }
        // Must be a push
        code()[bp.instruction_idx].arg1 = function_idxs[bp.label];
    }
}

std::string ByteCodeEmitter::toString() {
    return instructionsToString(code(), false);
}

void ByteCodeEmitter::loadIdentifier(const std::shared_ptr<AST::Identifier>& identifier) {

    auto it = std::find(localNames.begin(), localNames.end(), identifier->getName());
    if (it == localNames.end()) {
        throwConstraintViolated("Identifier not found in local names.");
    }
    auto localIdx = std::distance(localNames.begin(), it);
    code().push_back(executor::Instruction(executor::Op::LOCALL, localIdx)); // Bring it on the stack
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
            code().push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            break;
        }
        case AST::NodeType::Declvar: {
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            auto localIdx = localNames.size();
            code().push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            localNames.push_back(declvar->getIdentifier()->getName());
            break;
        }
        case AST::NodeType::StructAccess: {
            auto structAccess = std::dynamic_pointer_cast<AST::StructAccess>(node);
            ASSURE(structAccess->getIdentifiers().size() >= 2, "Struct access must have at least two identifiers");
            const auto& identifiers = structAccess->getIdentifiers();

            // Load first identifier
            auto identifierIt = identifiers.begin();
            ASSURE(identifierIt != identifiers.end(),
                   "StructAccess must have at least one identifier.");

            // Load address in first identifier, it is a local variable
            auto it = std::find(localNames.begin(), localNames.end(), (*identifierIt)->getName()); // TODO: This repeats, make function
            if (it == localNames.end()) {
                throwConstraintViolated("Identifier not found in local names.");
            }
            auto localIdx = std::distance(localNames.begin(), it);
            code().push_back(executor::Instruction(executor::Op::LOCALL, localIdx));
            // We got the address of the first struct on the stack

            const auto& type = (*identifierIt)->getDataType();
            ASSURE(type.isStruct(), "First identifier in StructAccess must be a struct type.");
            const auto* structType = &type.getStruct();
            ++identifierIt;

            // Load in-between identifiers
            while (identifierIt != std::prev(identifiers.end())) {
                const auto& name = (*identifierIt)->getName();
                auto fieldIt = structType->fields.find(name);
                ASSURE(fieldIt != structType->fields.end(), "Field not found in struct");
                const auto& field = fieldIt->second;
                ASSURE(field.type.isStruct(), "In-between identifier in StructAccess must be a struct type.");
                structType = &field.type.getStruct();
                code().push_back(executor::Instruction(executor::Op::LOADW, field.offset));
                ++identifierIt;
            }

            ASSURE(identifierIt != identifiers.end(),
                   "StructAccess must have at least one identifier after the first.");

            // Write to last identifier 
            const auto& name = (*identifierIt)->getName();
            auto fieldIt = structType->fields.find(name);
            ASSURE(fieldIt != structType->fields.end(), "Field not found in struct");
            const auto& field = fieldIt->second;
            // We don't really care about the type here, we just store a word
            code().push_back(executor::Instruction(executor::Op::STOREW, field.offset));
            break;
        }
        default:{
            std::cout << node->toString() << std::endl;
            throwConstraintViolated("Invalid LValue type.");
        }
    }

}

size_t ByteCodeEmitter::allocStructs(const DataType::Struct& structType) {
    size_t instructions = 0;

    code().push_back(executor::Instruction(executor::Op::ALLOC, structType.getMemorySize()));
    instructions++;
    for (const auto& [fieldName, member] : structType.fields) {
        if(member.type.isStruct()) {
            allocStructs(member.type.getStruct());
            code().push_back(executor::Instruction(executor::Op::DUB, instructions)); // Bring address here
            code().push_back(executor::Instruction(executor::Op::STOREW, member.offset));
        }
    }

    return instructions;
}

void ByteCodeEmitter::process(const std::shared_ptr<AST::Node>& node, bool hasConsumer) {
    switch(node->getType()) {
        case AST::NodeType::ExternFn: {
            auto externFn = std::dynamic_pointer_cast<AST::ExternFn>(node);

            // Add strings into the program data
            auto aLibIdx = program.data.addString(externFn->getLibrary());
            auto aNameIdx = program.data.addString(externFn->getName());

            ffi::qword_t returnTypeCode = 0;
            switch(externFn->getDataType().getReturn()->getPrimitive()){
                case DataType::Primitive::Int:
                    returnTypeCode = ffi::ret_type::Number;
                    break;
                case DataType::Primitive::Float:
                    returnTypeCode = ffi::ret_type::Float;
                    break;
                case DataType::Primitive::Bool:
                    returnTypeCode = ffi::ret_type::Bool;
                    break;
                case DataType::Primitive::Void:
                    returnTypeCode = ffi::ret_type::Void;
                    break;
                default:
                    throwConstraintViolated("Unsupported extern function return type.");
            }

            // Register the external function
            // This brings the function address on the stack
            code().push_back(executor::Instruction(executor::Op::REG_FFI, aLibIdx, aNameIdx, returnTypeCode));

            if(!hasConsumer) {
                // TODO: We might just skip creating the function at all
                code().push_back(executor::Instruction(executor::Op::POP));
            }
            break;
        }
        case AST::NodeType::Declfn:
        case AST::NodeType::Function: {
            // Should not happen, as it this runs on extracted functions
            throwConstraintViolated("Declfn should not be processed here.");
            break;
        }
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
            code().push_back(executor::Instruction(executor::Op::RET));
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
            auto jumpIfIdx = code().size();
            code().push_back(executor::Instruction(executor::Op::JUMP_IF, 0)); // Go to else or end
            process(ifNode->getPositive(), false);

            if (ifNode->getNegative()) {
                auto jumpEndIdx = code().size();
                code().push_back(executor::Instruction(executor::Op::JUMP, 0)); // Go to end
                process(ifNode->getNegative(), false);
                auto endIdx = code().size();
                code().push_back(executor::Instruction(executor::Op::NOP));
                code()[jumpIfIdx].arg1 = jumpEndIdx + 1; // Backpatch the jump if, skip to end
                code()[jumpEndIdx].arg1 = endIdx; // Backpatch the jump to end
            } else {
                auto endIdx = code().size();
                code().push_back(executor::Instruction(executor::Op::NOP));
                code()[jumpIfIdx].arg1 = endIdx; // Backpatch the jump if, skip to end
            }
            break;
        }
        case AST::NodeType::While: {
            auto whileNode = std::dynamic_pointer_cast<AST::While>(node);
            auto startIdx = code().size();
            process(whileNode->getCondition(), true);
            auto jumpIfIdx = code().size();
            code().push_back(executor::Instruction(executor::Op::JUMP_IF, 0)); // Go to end if false
            process(whileNode->getBody(), false);
            code().push_back(executor::Instruction(executor::Op::JUMP, startIdx)); // Jump back to condition
            auto endIdx = code().size();
            code().push_back(executor::Instruction(executor::Op::NOP));
            code()[jumpIfIdx].arg1 = endIdx; // Backpatch the jump if, skip to end
            break;
        }
        case AST::NodeType::Call: {
            auto call = std::dynamic_pointer_cast<AST::Call>(node);

            const auto& identifier = call->getIdentifier();
            ASSURE_NOT_NULL(identifier);

            const auto& fnDataType = identifier->getDataType();
            ASSURE(fnDataType.isFunction(), "Call identifier must be a function type.");
            const auto& functionType = fnDataType.getFunction();

            for(const auto& arg : call->getArguments()) {
               process(arg, true);
               if(functionType.isExtern){
                    code().push_back(executor::Instruction(executor::Op::PUSH_FFI_QWORD));
               }
            }

            const auto& fnName = identifier->getName();
            if(fnName == "+"){
                code().push_back(executor::Instruction(executor::Op::ADD));
            } else if (fnName == "-") {
                code().push_back(executor::Instruction(executor::Op::SUB));
            } else if (fnName == "*") {
                code().push_back(executor::Instruction(executor::Op::MUL));
            } else if (fnName == "/") {
                code().push_back(executor::Instruction(executor::Op::DIV));
            } else if (fnName == "%") {
                code().push_back(executor::Instruction(executor::Op::MOD));
            } else if (fnName == "<") {
                code().push_back(executor::Instruction(executor::Op::LT));
            } else if (fnName == ">") {
                code().push_back(executor::Instruction(executor::Op::GT));
            } else if (fnName == "==") {
                code().push_back(executor::Instruction(executor::Op::EQ));
            } else if (fnName == "<=") {
                code().push_back(executor::Instruction(executor::Op::LTE));
            } else if (fnName == ">=") {
                code().push_back(executor::Instruction(executor::Op::GTE));
            } else if (fnName == "!=") {
                code().push_back(executor::Instruction(executor::Op::NEQ));
            } else {
                loadIdentifier(call->getIdentifier()); // Bring the function addr on the stack

                const auto& returnType = fnDataType.getReturn();
                ASSURE_NOT_NULL(returnType);

                // CALL consumes the arguments from the stack, so we don't need to pop them
                // If we don't have a consumer, we have to pop the result. We assume only one result.
                // Only if the function is not void, of course.
                if (functionType.isExtern) {
                    code().push_back(executor::Instruction(executor::Op::CALL_FFI));

                    // CALL_FFI always pushes a result, so we need to get rid of it
                    // if we don't have a consumer or the return type is None.
                    if(!hasConsumer || *returnType == DataType::Primitive::None) {
                        code().push_back(executor::Instruction(executor::Op::POP));
                    }
                } else {
                    code().push_back(executor::Instruction(executor::Op::CALL, call->getArguments().size()));

                    if(!hasConsumer && *returnType != DataType::Primitive::None) {
                        code().push_back(executor::Instruction(executor::Op::POP));
                    }
                }
            }

            break;
        }
        case AST::NodeType::Literal: {
            if(hasConsumer) {
                auto literal = std::dynamic_pointer_cast<AST::Literal>(node);
                if (literal->getDataType() == DataType::Primitive::String) {
                    auto strIdx = program.data.addString(literal->getStringValue());
                    code().push_back(executor::Instruction(executor::Op::DATA_ADDR, strIdx));
                } else if (literal->getDataType() == DataType::Primitive::Bool) {
                    code().push_back(executor::Instruction(
                        executor::Op::PUSH, literal->getBoolValue() ? 1 : 0));
                }
                else if (literal->getDataType() == DataType::Primitive::Int) {
                    code().push_back(executor::Instruction(
                        executor::Op::PUSH, literal->getIntValue()));
                } else if (literal->getDataType() == DataType::Primitive::Float) {
                    code().push_back(executor::Instruction(
                        executor::Op::PUSH, literal->getFloatValue()));
                }
            }
            break;
        }
        case AST::NodeType::FnPtr: {
            if(hasConsumer) {
                auto fnPtr = std::dynamic_pointer_cast<AST::FnPtr>(node);
                // Later backpatch the address
                backpatches.push_back(Backpatch{code().size(), fnPtr->getId()});
                code().push_back(executor::Instruction(executor::Op::PUSH, 0));
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
                code().push_back(executor::Instruction(executor::Op::PUSH, 0)); // Initial value
                code().push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            } else if(dataType.isStruct()) {
                const auto& structType = dataType.getStruct();
                allocStructs(structType);
                code().push_back(executor::Instruction(executor::Op::LOCALS, localIdx));
            }
            break;
        }
        case AST::NodeType::DeclStruct: {
            // Are collected in TypesCollector
            break;
        }
        case AST::NodeType::StructAccess: {
            auto structAccess = std::dynamic_pointer_cast<AST::StructAccess>(node);
            ASSURE(structAccess->getIdentifiers().size() >= 2, "Struct access must have at least two identifiers");
            const auto& identifiers = structAccess->getIdentifiers();

            // Load first identifier
            auto identifierIt = identifiers.begin();
            ASSURE(identifierIt != identifiers.end(),
                   "StructAccess must have at least one identifier.");

            // Load address in first identifier, it is a local variable
            auto it = std::find(localNames.begin(), localNames.end(), (*identifierIt)->getName()); // TODO: This repeats, make function
            if (it == localNames.end()) {
                throwConstraintViolated("Identifier not found in local names.");
            }
            auto localIdx = std::distance(localNames.begin(), it);
            code().push_back(executor::Instruction(executor::Op::LOCALL, localIdx));
            // We got the address of the first struct on the stack

            const auto& type = (*identifierIt)->getDataType();
            ASSURE(type.isStruct(), "First identifier in StructAccess must be a struct type.");
            const auto* structType = &type.getStruct();
            ++identifierIt;

            // Load rest of identifiers
            while (identifierIt != std::prev(identifiers.end())) {
                const auto& name = (*identifierIt)->getName();
                auto fieldIt = structType->fields.find(name);
                ASSURE(fieldIt != structType->fields.end(), "Field not found in struct");
                const auto& field = fieldIt->second;
                ASSURE(field.type.isStruct(), "In-between identifier in StructAccess must be a struct type.");
                structType = &field.type.getStruct();
                code().push_back(executor::Instruction(executor::Op::LOADW, field.offset));
                ++identifierIt;
            }

            ASSURE(identifierIt != identifiers.end(),
                   "StructAccess must have at least one identifier after the first.");

            // Write to last identifier 
            const auto& name = (*identifierIt)->getName();
            auto fieldIt = structType->fields.find(name);
            ASSURE(fieldIt != structType->fields.end(), "Field not found in struct");
            const auto& field = fieldIt->second;

            // We don't really care about the type here, we just store a word
            code().push_back(executor::Instruction(executor::Op::LOADW, field.offset));
            break;
        }
    }
}

} // namespace emitter
