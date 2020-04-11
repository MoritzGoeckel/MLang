#pragma once

#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"

/*
 * TODO documentation
 */
class InfereIdentifierTypes {
   private:
    std::vector<std::map<std::string, DataType>> stack;

    // TODO: Maybe put in super class for TreeWalkers
    void followChildren(std::shared_ptr<AST::Node>& node) {
        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }
    }

   public:
    InfereIdentifierTypes() { stack.push_back({}); }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        // Block
        if (node->getType() == AST::NodeType::Block) {
            stack.push_back({});
            followChildren(node);
        }

        // Assignment
        else if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);

            // Determine right type
            process(assign->getRight());
            assign->getRight()->infereDataType();

            // Assign left type
            if (assign->getLeft()->getType() == AST::NodeType::Declvar) {
                // Variable declaration, use right side type
                auto declvar =
                    std::dynamic_pointer_cast<AST::Declvar>(assign->getLeft());
                auto rightType = assign->getRight()->getDataType();
                declvar->hintDataType(rightType);

                auto name = declvar->getIdentifier()->getName();
                if (stack.back().find(name) != stack.back().end()) {
                    // TODO: Emit messages
                    std::cout << "-- Variable already declared " << name
                              << std::endl;
                }
                stack.back().emplace(name, rightType);

            } else if (assign->getLeft()->getType() == AST::NodeType::Declfn) {
                // TODO function declaration, use right return type
                // PUSH function type to stack
                // name -> [arg_types] -> type
                // Need standard functions + - * / etc
                // E.g. + -> [int, int] -> int
            } else if (assign->getLeft()->getType() ==
                       AST::NodeType::Identifier) {
                process(assign->getLeft());
            } else {
                // TODO: Emit messages
                std::cout << "-- Assigning to other than declvar, declfn or "
                          << "identifier" << std::endl;
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
                identifier->hintDataType(type);
            } else {
                // TODO: Emit messages
                std::cout << "-- Undeclared variable " << name << std::endl;
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

            // TODO: Use parameter types and name to determine type
            // hintDataType(/* type */); TODO
        }

        // Any other node
        else {
            followChildren(node);
        }

        return node;
    }
};
