#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

class ApplyTypeAnnotations : private TreeWalker {
   public:
    ApplyTypeAnnotations();
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
};
