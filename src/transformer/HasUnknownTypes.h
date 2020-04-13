#pragma once

#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"

/*
 * TODO documentation
 */
class HasUnknownTypes {
   private:
    // TODO: Maybe put in super class for TreeWalkers
    void followChildren(std::shared_ptr<AST::Node>& node) {
        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }
    }

    size_t unresolvedNodes;

   public:
    HasUnknownTypes() : unresolvedNodes(0u) {}

    bool isAllTypesResolved() { return unresolvedNodes == 0u; }
    size_t getNumberOfUnresolvedNodes() { return unresolvedNodes; }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        if (node->getDataType() == DataType::Primitive::Unknown) {
            ++unresolvedNodes;
        }
        followChildren(node);
        return node;
    }
};
