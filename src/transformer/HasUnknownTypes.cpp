#include "HasUnknownTypes.h"

std::shared_ptr<AST::Node> HasUnknownTypes::process(
    std::shared_ptr<AST::Node> node) {
    if (node->getDataType() == DataType::Primitive::Unknown) {
        ++unresolvedNodes;

        std::string aMsg = "Unresolved type";

        auto aIdentifier = std::dynamic_pointer_cast<AST::Identifier>(node);
        if (aIdentifier) aMsg += " for '" + aIdentifier->getName() + "'";

        itsErrors.emplace_back(aMsg, node->getPosition());
    }

    if (node->getDataType() == DataType::Primitive::Conflict) {
        ++typeConflicts;

        std::string aMsg = "Conflicting type";

        auto aIdentifier = std::dynamic_pointer_cast<AST::Identifier>(node);
        if (aIdentifier) aMsg += " for '" + aIdentifier->getName() + "'";

        itsErrors.emplace_back(aMsg, node->getPosition());
    }

    followChildren(node);
    return node;
}
