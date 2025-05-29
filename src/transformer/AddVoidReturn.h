#pragma once

#include "../ast/Node.h"

namespace transformer {
class AddVoidReturn {
   public:
    AddVoidReturn();
    std::shared_ptr<AST::Function> process(const std::shared_ptr<AST::Function>& node);
};
} // namespace transformer
