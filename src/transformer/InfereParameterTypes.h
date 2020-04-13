#pragma once

#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"

/*
 * TODO documentation
 */
class InfereParameterTypes {
   private:
    // TODO: Should support overloading
    // Stack of (fn ids -> parameters)
    std::vector<
        std::map<std::string, std::vector<std::shared_ptr<AST::Identifier>>>>
        stack;

    // TODO: Maybe put in super class for TreeWalkers
    void followChildren(std::shared_ptr<AST::Node>& node) {
        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }
    }

   public:
    InfereParameterTypes() { stack.push_back({}); }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        // Block
        if (node->getType() == AST::NodeType::Block) {
            stack.push_back({});
            followChildren(node);
        }

        // Declfn
        else if (node->getType() == AST::NodeType::Declfn) {
            auto declfn = std::dynamic_pointer_cast<AST::Declfn>(node);
            auto name = declfn->getIdentifier()->getName();

            std::vector<std::shared_ptr<AST::Identifier>> parameters;
            parameters.reserve(declfn->getParameters().size());
            for (auto& ident : declfn->getParameters()) {
                parameters.push_back(ident);
            }
            stack.back().emplace(name, parameters);
        }

        // Call
        else if (node->getType() == AST::NodeType::Call) {
            // Gather arument types, find closest function on stack, hint
            // parameter types to its parameters

            auto call = std::dynamic_pointer_cast<AST::Call>(node);
            auto name = call->getIdentifier()->getName();
            auto& arguments = call->getArguments();
            std::vector<std::shared_ptr<AST::Identifier>>* parameters = nullptr;

            // Stop if identifier already has types
            if (call->getIdentifier()->getDataType() !=
                DataType::Primitive::Unknown) {
                // Type already determined
                return node;
            }

            for (auto rIt = stack.rbegin(); rIt != stack.rend(); ++rIt) {
                // TODO: Use parameter types AND name to determine type
                // Not only name. To enable overloading
                if (rIt->find(name) != rIt->end()) {
                    parameters = &((*rIt)[name]);
                    break;
                }
            }

            if (!parameters) {
                // TODO: Emit messages
                std::cout << "-- Undeclared function " << name << std::endl;
                // TODO: This gives false warnings for build-in functions

                followChildren(node);
                return node;
            }

            if (parameters->size() != arguments.size()) {
                // TODO: Emit messages
                std::cout
                    << "-- Function call with unexpected number of arguments "
                    << name << " declared: " << parameters->size()
                    << " provided: " << arguments.size() << std::endl;

                followChildren(node);
                return node;
            }

            for (size_t i = 0u; i < arguments.size(); ++i) {
                const auto& type = arguments[i]->getDataType();
                if (type != DataType::Primitive::Unknown &&
                    type != DataType::Primitive::Conflict) {
                    parameters->at(i)->hintDataType(type);
                }
            }
        }

        // Any other node
        else {
            followChildren(node);
        }

        return node;
    }
};
