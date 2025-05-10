#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/Exceptions.h"

namespace emitter {

class Emitter {
   protected:
    std::map<std::string, std::shared_ptr<AST::Function>> functions;
    size_t lastId;

   public:
    Emitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions);
    virtual ~Emitter() = default;

    void print();

    virtual void run() = 0;
    virtual std::string toString() = 0;

   protected:
    std::string createUnique(const char *str);
    std::string createUnique(std::string str);

    virtual void process(const std::shared_ptr<AST::Node>& node) = 0;
    void followChildren(const std::shared_ptr<AST::Node>& node);
};

} // namespace emitter