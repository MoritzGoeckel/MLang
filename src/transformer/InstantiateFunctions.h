#pragma once

#include <iostream>
#include <memory>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "TreeWalker.h"

/*
 * TODO, doc
 * Search for assignments with declfns.
 * Instantiate fn body
 * Replace declfn with declvar. Name is same, value is unique name
 */
class InstantiateFunctions : private TreeWalker {
   private:
    std::map<std::string, std::shared_ptr<AST::Function>> functions;
    size_t depth;

   public:
    InstantiateFunctions(std::shared_ptr<AST::Node> node) : depth(0) {
        // Instantiate functions
        process(node);

        // Instantiate main
        auto declfn = std::make_shared<AST::Declfn>("main");
        auto addMsg = [this](auto& s) { addMessage(s); };
        declfn->getIdentifier()->setDataType(
            DataType({}, node->getReturnType(addMsg)), addMsg);

        auto fn = std::make_shared<AST::Function>(declfn, node);
        functions["main"] = fn;
    }

    std::map<std::string, std::shared_ptr<AST::Function>>& getFunctions() {
        return functions;
    }

   private:
    std::shared_ptr<AST::Node> process(std::shared_ptr<AST::Node> node) {
        depth++;
        followChildren(node);
        depth--;

        if (node->getType() == AST::NodeType::Assign) {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            if (assign->getLeft()->getType() == AST::NodeType::Declfn) {
                auto declfn =
                    std::dynamic_pointer_cast<AST::Declfn>(assign->getLeft());

                // Create unique id
                std::string uniqId(
                    std::to_string(depth) + "_" +
                    declfn->getIdentifier()->getName() + "_" +
                    declfn->getIdentifier()->getDataTypeString());

                // Instantiate
                // TODO: Assert its not already in
                auto fn =
                    std::make_shared<AST::Function>(declfn, assign->getRight());
                functions[uniqId] = fn;

                // Update assignment
                assign->setLeft(
                    std::make_shared<AST::Declvar>(declfn->getIdentifier()));
                assign->setRight(std::make_shared<AST::FnPtr>(
                    uniqId, declfn->getIdentifier()->getDataType()));
            }
        }

        return node;
    }
};
