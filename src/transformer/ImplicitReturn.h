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
    std::shared_ptr<AST::Node>& process(std::shared_ptr<AST::Node>& node) {
        if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            if (assign->getLeft()->getType() == AST::NodeType::Declfn &&
                assign->getRight()->getType() != AST::NodeType::Block &&
                assign->getRight()->getType() != AST::NodeType::Ret) {

                auto ret = std::make_shared<AST::Ret>(assign->getRight());
                assign->setRight(ret);
            }
        }

        // Adds return to blocks with only one statement. Not active
        /*else if (node->getType() == AST::NodeType::Block){
            auto block = std::dynamic_pointer_cast<AST::Block>(node);
            auto children = block->getChildren();
            if(children.size() == 1u
               && children.front()->getType() != AST::NodeType::Ret){
                auto ret = std::make_shared<AST::Ret>(children.front());
                block->setChildren({ret});
            }
        }*/

        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }

        return node;
    }
};
