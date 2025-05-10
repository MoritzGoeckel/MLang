#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TreeWalker to infere types of identifiers throughout the different scopes
 */
class InfereIdentifierTypes : private TreeWalker {
   private:
    std::vector<std::map<std::string, DataType>> stack;

   public:
    InfereIdentifierTypes();
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
};
