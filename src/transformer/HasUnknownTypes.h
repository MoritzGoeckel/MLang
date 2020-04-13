#pragma once

#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TreeWalker that checks whether any types in the ast are unknown or
 * conflicting. Use messages or the getters to receive the result.
 */
class HasUnknownTypes : TreeWalker {
   private:
    size_t unresolvedNodes;
    size_t typeConflicts;

   public:
    HasUnknownTypes() : unresolvedNodes(0u), typeConflicts(0u) {}

    bool isAllTypesResolved() {
        return unresolvedNodes == 0u && typeConflicts == 0u;
    }
    bool hasTypeConflicts() { return typeConflicts != 0u; }

    size_t getNumberOfUnresolvedNodes() { return unresolvedNodes; }
    size_t getNumberOfTypeConflicts() { return typeConflicts; }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
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
};
