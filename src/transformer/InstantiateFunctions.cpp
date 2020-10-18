#include "InstantiateFunctions.h"

InstantiateFunctions::InstantiateFunctions(std::shared_ptr<AST::Node> node)
    : depth(0) {
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

std::map<std::string, std::shared_ptr<AST::Function>>&
InstantiateFunctions::getFunctions() {
    return functions;
}

std::shared_ptr<AST::Node> InstantiateFunctions::process(
    std::shared_ptr<AST::Node> node) {
    depth++;
    followChildren(node);
    depth--;

    if (node->getType() == AST::NodeType::Assign) {
        auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
        if (assign->getLeft()->getType() == AST::NodeType::Declfn) {
            auto declfn =
                std::dynamic_pointer_cast<AST::Declfn>(assign->getLeft());

            // Create unique id
            std::string uniqId(std::to_string(depth) + "_" +
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
