#pragma once

#include <memory>
#include <vector>

#include "../ast/Node.h"
#include "../error/TypeError.h"

namespace validator {

/*
 * Base class for validators that check properties of functions.
 * Validators can accumulate errors and provide methods to check for them.
 */
class Validator {
   protected:
    std::vector<TypeError> itsErrors;

   public:
    Validator() : itsErrors() {}

    virtual ~Validator() = default;

    const std::vector<TypeError>& getErrors() const { return itsErrors; }

    bool hasErrors() const { return !itsErrors.empty(); }

    // Process a function and validate it
    virtual void validate(std::shared_ptr<AST::Function> function) = 0;
};

}  // namespace validator
