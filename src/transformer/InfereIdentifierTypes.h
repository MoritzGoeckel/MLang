#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TreeWalker to infere types of identifiers throughout the different scopes
 */
class InfereIdentifierTypes : TreeWalker {
   private:
    std::vector<std::map<std::string, DataType>> stack;

   public:
    InfereIdentifierTypes() {
        stack.push_back({});

#define DEF_BUILD_IN(NAME, TYPE)                                               \
    stack.back().emplace(                                                      \
        NAME, DataType({DataType::Primitive::TYPE, DataType::Primitive::TYPE}, \
                       DataType::Primitive::TYPE))

        // Add std function types in first stack frame
        DEF_BUILD_IN("-", Int);
        DEF_BUILD_IN("+", Int);
        DEF_BUILD_IN("/", Int);
        DEF_BUILD_IN("*", Int);
        DEF_BUILD_IN("<", Int);
        DEF_BUILD_IN(">", Int);
        DEF_BUILD_IN(">=", Int);
        DEF_BUILD_IN("<=", Int);
        DEF_BUILD_IN("||", Bool);
        DEF_BUILD_IN("&&", Bool);

        stack.push_back({});
    }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        // Block
        if (node->getType() == AST::NodeType::Block) {
            stack.push_back({});
            followChildren(node);
            stack.pop_back();
        }

        // Assignment
        else if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);

            // Assignment with variable declaration left
            if (assign->getLeft()->getType() == AST::NodeType::Declvar) {
                // Evaluate right side first
                process(assign->getRight());

                // Use right side type for the left side type
                auto declvar =
                    std::dynamic_pointer_cast<AST::Declvar>(assign->getLeft());
                auto rightType = assign->getRight()->getDataType();
                declvar->getIdentifier()->setDataType(
                    rightType, [this](auto& s) { addMessage(s); });

                auto name = declvar->getIdentifier()->getName();
                if (stack.back().find(name) != stack.back().end()) {
                    addMessage("Variable already declared: " + name);
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
                    stack.back().emplace(ident->getName(),
                                         ident->getDataType());
                }

                // Evaluate right side
                process(assign->getRight());
                auto retType = assign->getRight()->getReturnType(
                    [this](auto& s) { addMessage(s); });

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
                        determinedFnType, [this](auto& s) { addMessage(s); });
                    stack.back().emplace(declfn->getIdentifier()->getName(),
                                         DataType(paramTypes, retType));
                }

            } else if (assign->getLeft()->getType() ==
                       AST::NodeType::Identifier) {
                process(assign->getRight());
                process(assign->getLeft());
            } else {
                addMessage(
                    "Assigning to other than declvar, declfn or identifier");
            }
        }

        // Identifier
        else if (node->getType() == AST::NodeType::Identifier) {
            // Existing var, get type from stack
            auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
            auto name = identifier->getName();

            DataType type = DataType::Primitive::Unknown;
            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                if (rIt->find(name) != rIt->end()) {
                    type = (*rIt)[name];
                    break;
                }
            }

            if (type != DataType::Primitive::Unknown) {
                identifier->setDataType(type,
                                        [this](auto& s) { addMessage(s); });
            } else {
                addMessage("Undeclared variable: " + name);
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

            if (type != DataType::Primitive::Unknown) {
                call->getIdentifier()->setDataType(
                    type, [this](auto& s) { addMessage(s); });
            } else {
                addMessage("Undeclared variable: " + name);
            }
        }

        // Any other node
        else {
            followChildren(node);
        }

        return node;
    }
};
