#pragma once

#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TreeWalker that inferes the types of function parameters. This is done by
 * scanning for function calls. May emit false error messages for build-in
 * functions.
 */
class InfereParameterTypes : TreeWalker {
   private:
    // TODO: Should support overloading
    // Stack of (fn ids -> parameters)
    std::vector<
        std::map<std::string, std::vector<std::shared_ptr<AST::Identifier>>>>
        stack;

   public:
    InfereParameterTypes();

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
};
