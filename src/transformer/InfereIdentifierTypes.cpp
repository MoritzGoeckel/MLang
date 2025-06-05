#include "InfereIdentifierTypes.h"

#include <algorithm>

InfereIdentifierTypes::InfereIdentifierTypes() {
    stack.push_back({});

#define DEF_BUILD_IN(NAME, TYPE)                                               \
    stack.back().emplace(                                                      \
        NAME, DataType({DataType::Primitive::TYPE, DataType::Primitive::TYPE}, \
                       DataType::Primitive::TYPE))

#define DEF_BUILD_IN_2(NAME, TYPE, RTYPE)                                      \
    stack.back().emplace(                                                      \
        NAME, DataType({DataType::Primitive::TYPE, DataType::Primitive::TYPE}, \
                       DataType::Primitive::RTYPE))

    // Add std function types in first stack frame
    DEF_BUILD_IN("-", Int);
    DEF_BUILD_IN("+", Int);
    DEF_BUILD_IN("/", Int);
    DEF_BUILD_IN("*", Int);

    DEF_BUILD_IN_2("<", Int, Bool);
    DEF_BUILD_IN_2(">", Int, Bool);
    DEF_BUILD_IN_2(">=", Int, Bool);
    DEF_BUILD_IN_2("<=", Int, Bool);

    DEF_BUILD_IN("||", Bool);
    DEF_BUILD_IN("&&", Bool);

    stack.push_back({});
}

std::shared_ptr<AST::Node> InfereIdentifierTypes::process(std::shared_ptr<AST::Node> node) {
    // Block
    if (node->getType() == AST::NodeType::Block) {
        stack.push_back({});
        followChildren(node);
        stack.pop_back();
    }

    // Declvar
    else if (node->getType() == AST::NodeType::Declvar) {
        auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
        auto identifier = declvar->getIdentifier();

        auto name = identifier->getName();
        if (stack.back().find(name) != stack.back().end()) {
            this->addMessage("Variable already declared: " + name);
        }
        stack.back().emplace(name, identifier->getDataType());
    }

    // Declstruct
    else if (node->getType() == AST::NodeType::DeclStruct) {
        auto declstruct = std::dynamic_pointer_cast<AST::DeclStruct>(node);
        auto identifier = declstruct->getIdentifier();
        ASSURE_NOT_NULL(identifier);
        identifier->setDataType(
            DataType::Primitive::Struct, [this](auto& s) { this->addMessage(s); });
        followChildren(node);
    }

    // Assignment
    else if (node->getType() == AST::NodeType::Assign) {
        auto assign = std::dynamic_pointer_cast<AST::Assign>(node);

        if(assign->getLeft()->getType() == AST::NodeType::StructAccess){
            process(assign->getLeft());
        }

        // Assignment with variable declaration left
        if (assign->getLeft()->getType() == AST::NodeType::Declvar) {
            // Evaluate right side first
            process(assign->getRight());

            // Use right side type for the left side type
            auto declvar =
                std::dynamic_pointer_cast<AST::Declvar>(assign->getLeft());
            auto rightType = assign->getRight()->getDataType();
            declvar->getIdentifier()->setDataType(
                rightType, [this](auto& s) { this->addMessage(s); });

            auto name = declvar->getIdentifier()->getName();
            if (stack.back().find(name) != stack.back().end()) {
                this->addMessage("Variable already declared: " + name);
            }
            stack.back().emplace(name, rightType);
        }

        // Assignment with function declaration left
        else if (assign->getLeft()->getType() == AST::NodeType::Declfn) {
            auto declfn =
                std::dynamic_pointer_cast<AST::Declfn>(assign->getLeft());
            std::vector<DataType> paramTypes;
            paramTypes.reserve(declfn->getParameters().size());

            // Push params to stack
            stack.push_back({});

            for (auto& ident : declfn->getParameters()) {
                paramTypes.push_back(ident->getDataType());
                stack.back().emplace(ident->getName(), ident->getDataType());
            }

            // Evaluate right side
            process(assign->getRight());
            auto retType = assign->getRight()->getReturnType(
                [this](auto& s) { this->addMessage(s); });

            // Pop stack with parameters after evaluating right side
            stack.pop_back();

            // TODO: Identify not only by name, but also by parameter
            // types Else overloading is not possible

            // Only fill if parameter types are certain
            if (std::none_of(paramTypes.begin(), paramTypes.end(),
                             [](DataType& t) {
                                 return t == DataType::Primitive::Unknown;
                             })) {
                auto determinedFnType = DataType(paramTypes, retType);
                declfn->getIdentifier()->setDataType(
                    determinedFnType, [this](auto& s) { this->addMessage(s); });
                stack.back().emplace(declfn->getIdentifier()->getName(),
                                     DataType(paramTypes, retType));
            }

        } else if (assign->getLeft()->getType() == AST::NodeType::Identifier) {
            process(assign->getRight());
            process(assign->getLeft());
        } else {
            this->addMessage("Assigning to other than declvar, declfn or identifier");
        }
    }

    // Struct access
    else if(node->getType() == AST::NodeType::StructAccess){
        auto structAccess = std::dynamic_pointer_cast<AST::StructAccess>(node);
        ASSURE(structAccess->getIdentifiers().size() >= 2, "Struct access must have at least two identifier");
        const auto& identifiers = structAccess->getIdentifiers();

        auto getType = [this](const std::string& name) -> DataType {
            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                if (rIt->find(name) != rIt->end()) {
                    return (*rIt)[name];
                }
            }
            return DataType::Primitive::Unknown;
        };

        const auto& first = identifiers.front();
        const auto& name = first->getName();
        auto lastType = getType(name);
        if (lastType != DataType::Primitive::Unknown) {
            first->setDataType(lastType, [this](auto& s) { this->addMessage(s); });

            for(auto current = std::next(identifiers.begin()); current != identifiers.end(); ++current) {
                auto currentIdentifier = *current;
                ASSURE(lastType.isStruct(), "First identifier must be a struct");
                const auto& parentFields = lastType.getStruct().fields;
                auto it = parentFields.find(currentIdentifier->getName());
                ASSURE(it != parentFields.end(), "Field not found in struct");
                auto currentType = it->second;

                if (currentType != DataType::Primitive::Unknown) {
                    currentIdentifier->setDataType(currentType, [this](auto& s) { this->addMessage(s); });
                } else {
                    this->addMessage("Undeclared field: " + currentIdentifier->getName());
                }
                lastType = currentType;
            }

            structAccess->setDataType(lastType, [this](auto& s) { this->addMessage(s); });
        } else {
            this->addMessage("Undeclared variable: " + name);
        }
    }

    // Identifier
    else if (node->getType() == AST::NodeType::Identifier) {
        // Existing var, get type from stack
        auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
        auto name = identifier->getName();

        // TODO: Use getType() to get type from stack
        DataType type = DataType::Primitive::Unknown;
        for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
            if (rIt->find(name) != rIt->end()) {
                type = (*rIt)[name];
                break;
            }
        }

        if (type != DataType::Primitive::Unknown) {
            identifier->setDataType(type, [this](auto& s) { this->addMessage(s); });
        } else {
            this->addMessage("Undeclared variable: " + name);
        }
    }

    // Function call
    else if (node->getType() == AST::NodeType::Call) {
        auto call = std::dynamic_pointer_cast<AST::Call>(node);

        // Get argument types
        std::vector<DataType> argumentTypes;
        for (auto& argument : call->getArguments()) {
            process(argument);
            argumentTypes.push_back(argument->getDataType());
        }
        auto name = call->getIdentifier()->getName();

        DataType type = DataType::Primitive::Unknown;
        for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
            // TODO: Use parameter types AND name to determine type
            // Not only name
            if (rIt->find(name) != rIt->end()) {
                type = (*rIt)[name];
                break;
            }
        }

        // TODO: Central place. This is for print. Build-ins
        if (name == "print" && argumentTypes.empty()) {
            type = DataType({}, DataType::Primitive::None);  // None?
        }

        if (type != DataType::Primitive::Unknown) {
            call->getIdentifier()->setDataType(
                type, [this](auto& s) { this->addMessage(s); });
        } else {
            this->addMessage("Undeclared variable: " + name);
        }
    }

    // Any other node
    else {
        followChildren(node);
    }

    return node;
}

