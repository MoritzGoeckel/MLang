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
    CollectTypes();

    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node);

    const std::map<std::string, DataType>& getTypes() const {
        return types;
    }

    private:
    std::map<std::string, DataType> types;
};
