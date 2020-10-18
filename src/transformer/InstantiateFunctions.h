#pragma once

#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TODO, doc
 * Search for assignments with declfns.
 * Instantiate fn body
 * Replace declfn with declvar. Name is same, value is unique name
 */
class InstantiateFunctions : private TreeWalker {
   private:
    std::map<std::string, std::shared_ptr<AST::Function>> functions;
    size_t depth;

   public:
    InstantiateFunctions(std::shared_ptr<AST::Node> node);
    std::map<std::string, std::shared_ptr<AST::Function>>& getFunctions();

   private:
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
};
