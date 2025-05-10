#include "InfereParameterTypes.h"

InfereParameterTypes::InfereParameterTypes() { stack.push_back({}); }

std::shared_ptr<AST::Node> InfereParameterTypes::process(
    std::shared_ptr<AST::Node> node) {
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

        // Stop if identifier already has types. This may happen when the
        // code already got infered or if we are looking at build-in
        // functions
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
            // May give false warnings for build-in functions
            addMessage("Undeclared function " + name);
            followChildren(node);
            return node;
        }

        if (parameters->size() != arguments.size()) {
            addMessage("Function call with unexpected number of arguments " +
                       name +
                       " declared: " + std::to_string(parameters->size()) +
                       " provided: " + std::to_string(arguments.size()));

            followChildren(node);
            return node;
        }

        for (size_t i = 0u; i < arguments.size(); ++i) {
            const auto& type = arguments[i]->getDataType();
            if (type != DataType::Primitive::Unknown &&
                type != DataType::Primitive::Conflict) {
                parameters->at(i)->setDataType(
                    type, [this](auto& s) { this->addMessage(s); });
            }
        }
    }

    // Any other node
    else {
        followChildren(node);
    }

    return node;
}
