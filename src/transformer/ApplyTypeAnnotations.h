#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/TypeError.h"
#include "TreeWalker.h"
#include "CollectTypes.h"

class ApplyTypeAnnotations : private TreeWalker {
   public:
    ApplyTypeAnnotations(CollectTypes::TypesMap& types);
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);
    const std::vector<TypeError>& getErrors() const { return itsErrors; }
    bool hasErrors() const { return !itsErrors.empty(); }
    void clearErrors() { itsErrors.clear(); }

   private:
    CollectTypes::TypesMap& types;
    std::vector<TypeError> itsErrors;
};
