#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"
#include "CollectTypes.h"

class ApplyTypeAnnotations : private TreeWalker {
   public:
    ApplyTypeAnnotations(CollectTypes::TypesMap& types);
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
    private:
    CollectTypes::TypesMap& types;
};
