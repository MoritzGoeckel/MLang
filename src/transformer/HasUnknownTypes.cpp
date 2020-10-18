#include "HasUnknownTypes.h"

std::shared_ptr<AST::Node> HasUnknownTypes::process(
    std::shared_ptr<AST::Node> node) {
    if (node->getDataType() == DataType::Primitive::Unknown) {
        ++unresolvedNodes;
        addMessage("Unresolved type: " + node->toString());
    }

    if (node->getDataType() == DataType::Primitive::Conflict) {
        ++typeConflicts;
        addMessage("Conflicting types: " + node->toString());
    }

    followChildren(node);
    return node;
}
