#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/Exceptions.h"
#include "TreeWalker.h"

/*
 * TODO
 */
class Emitter {
   private:
    std::map<std::string, std::shared_ptr<AST::Function>> functions;
    size_t lastId;

   public:
    Emitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions);

    ~Emitter();

    void instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast);

    void run();

    void print();

   private:
    std::string createUnique(const char *str);
    std::string createUnique(std::string str);

    void process(std::shared_ptr<AST::Node> node);

    void followChildren(std::shared_ptr<AST::Node> &node);
};
