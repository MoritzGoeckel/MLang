#pragma once

#include <iostream>
#include <memory>

#include "../ast/Node.h"

class ImplicitReturn {
   public:
    std::shared_ptr<AST::Node>& process(std::shared_ptr<AST::Node>& node) {
        if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            if (assign->getLeft()->getType() == AST::NodeType::Declfn &&
                assign->getRight()->getType() != AST::NodeType::Block &&
                assign->getRight()->getType() != AST::NodeType::Ret) {
                // TODO: Make return
                std::cout << std::endl << "! heeeeeyr !" << std::endl;
            }
        }

        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }

        return node;
    }
};
