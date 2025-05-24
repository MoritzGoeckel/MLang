#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/Exceptions.h"
#include "../transformer/TreeWalker.h"
#include "../executer/ByteCode.h"

#include "Emitter.h"

namespace emitter {

class ByteCodeEmitter : public Emitter {
    
    private:
    std::vector<executor::word_t> data;
    std::vector<executor::Instruction> code;
    std::map<std::string, executor::word_t> labels;

    public:
    ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions);

    void instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast);

    virtual void run();
    virtual std::string toString();

   protected:
    virtual void process(const std::shared_ptr<AST::Node>& node);
};

} // namespace emitter