#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

class CollectTypes : private TreeWalker {
    public:
    using TypesMap = std::map<std::string, DataType>;
    CollectTypes(TypesMap& types);

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);

    private:
    TypesMap& types;
};
