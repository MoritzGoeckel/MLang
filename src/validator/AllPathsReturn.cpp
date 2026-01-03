#include "AllPathsReturn.h"

namespace validator {

bool AllPathsReturn::allPathsReturn(std::shared_ptr<AST::Node> node) {
    if (!node) {
        return false;
    }

    auto nodeType = node->getType();

    // A return statement always returns
    if (nodeType == AST::NodeType::Ret) {
        return true;
    }

    // A block returns if any of its statements returns
    // (and it's the last statement, which we check by looking for returns anywhere)
    if (nodeType == AST::NodeType::Block) {
        auto block = std::dynamic_pointer_cast<AST::Block>(node);
        auto children = block->getChildren();

        // Check each statement in the block
        for (const auto& child : children) {
            if (child && allPathsReturn(child)) {
                // Found a return statement - this path returns
                return true;
            }
        }
        return false;
    }

    // An if statement returns only if:
    // 1. It has an else clause (bodyNegative is not null)
    // 2. Both branches return
    if (nodeType == AST::NodeType::If) {
        auto ifNode = std::dynamic_pointer_cast<AST::If>(node);
        auto positive = ifNode->getPositive();
        auto negative = ifNode->getNegative();

        // Must have both branches
        if (!negative) {
            return false;
        }

        // Both branches must return
        return allPathsReturn(positive) && allPathsReturn(negative);
    }

    // While loops don't guarantee execution, so they don't guarantee return
    if (nodeType == AST::NodeType::While) {
        return false;
    }

    // All other node types don't guarantee a return
    return false;
}

bool containsNonVoidReturn(std::shared_ptr<AST::Node> node) {
    if (!node) {
        return false;
    }

    if (node->getType() == AST::NodeType::Ret) {
        auto retNode = std::dynamic_pointer_cast<AST::Ret>(node);
        if (retNode->getExpr()) {
            // If the return has an expression, it's a non-void return
            return true;
        }
    }

    for (auto& child : node->getChildren()) {
        if (containsNonVoidReturn(child)) {
            return true;
        }
    }
    return false;
}

void AllPathsReturn::validate(std::shared_ptr<AST::Function> function) {
    auto body = function->getBody();
    auto head = function->getHead();

    // Only validate if the function returns a VALUE (non-void)
    // Functions that return void can have partial returns (AddVoidReturn will fix them)
    if (containsNonVoidReturn(body)) {
        if (!allPathsReturn(body)) {
            std::string functionName = head->getIdentifier()->getName();
            std::string msg = "Function '" + functionName +
                "' does not return a value on all code paths";
            itsErrors.emplace_back(msg, function->getPosition());
        }
    }
}

}  // namespace validator
