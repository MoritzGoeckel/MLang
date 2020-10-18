#include "TreeWalker.h"

std::string Message::toString() { return msg; }

void TreeWalker::addMessage(const Message& msg) { messages.emplace_back(msg); }

void TreeWalker::followChildren(std::shared_ptr<AST::Node>& node) {
    for (auto child : node->getChildren()) {
        if (!child) continue;
        process(child);
    }
}

std::vector<Message> TreeWalker::getMessages() { return messages; }

bool TreeWalker::hasMessages() { return !getMessages().empty(); }
