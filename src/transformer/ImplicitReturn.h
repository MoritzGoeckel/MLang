#pragma once

#include <iostream>
#include <memory>

#include "../ast/Node.h"

/*
 * Adds an return statement to function declarations if they
 * consist only of one statement
 */
class ImplicitReturn {
   public:
    std::shared_ptr<AST::Node>& process(std::shared_ptr<AST::Node>& node);
};
