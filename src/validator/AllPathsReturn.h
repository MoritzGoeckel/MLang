#pragma once

#include <memory>

#include "../ast/Node.h"
#include "Validator.h"

namespace validator {

/*
 * Validator that checks if all code paths in a non-void function return a value.
 * Reports an error if a function has branches that don't return.
 */
class AllPathsReturn : public Validator {
   private:
    bool allPathsReturn(std::shared_ptr<AST::Node> node);

   public:
    AllPathsReturn() : Validator() {}

    void validate(std::shared_ptr<AST::Function> function) override;
};

}  // namespace validator
