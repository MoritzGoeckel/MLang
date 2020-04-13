#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../ast/DataType.h"
#include "../ast/Node.h"

/*
 * Base class for error / warning messages used in TreeWalkers
 */
class Message {
   private:
    std::string msg;

   public:
    Message(const std::string& msg) : msg(msg) {}
    Message(const char* msg) : msg(msg) {}
    std::string toString() { return msg; }
};

/*
 * Base class for TreeWalkers that traverse abstract syntax trees. Contains
 * utility functions for traversal, a standardized entry method and basic
 * error message handling
 */
class TreeWalker {
   private:
    std::vector<Message> messages;

   protected:
    void addMessage(const Message& msg) { messages.emplace_back(msg); }
    void followChildren(std::shared_ptr<AST::Node>& node) {
        for (auto child : node->getChildren()) {
            if (!child) continue;
            process(child);
        }
    }

   public:
    // Standardized entry for TreeWalkers
    virtual std::shared_ptr<AST::Node> process(
        std::shared_ptr<AST::Node> node) = 0;

    virtual std::vector<Message> getMessages() { return messages; }

    virtual bool hasMessages() { return !getMessages().empty(); }
};
