#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/TypeError.h"
#include "TreeWalker.h"

/*
 * TreeWalker that checks whether any types in the ast are unknown or
 * conflicting. Use messages or the getters to receive the result.
 */
class HasUnknownTypes : TreeWalker {
   private:
    size_t unresolvedNodes;
    size_t typeConflicts;
    std::vector<TypeError> itsErrors;

   public:
    HasUnknownTypes() : unresolvedNodes(0u), typeConflicts(0u), itsErrors() {}

    void reset() {
        unresolvedNodes = 0u;
        typeConflicts = 0u;
        itsErrors.clear();
    }

    const std::vector<TypeError>& getErrors() { return itsErrors; }

    bool isAllTypesResolved() {
        return unresolvedNodes == 0u && typeConflicts == 0u;
    }
    bool hasTypeConflicts() { return typeConflicts != 0u; }

    size_t getNumUnresolved() { return unresolvedNodes; }
    size_t getNumConflicts() { return typeConflicts; }

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
};
