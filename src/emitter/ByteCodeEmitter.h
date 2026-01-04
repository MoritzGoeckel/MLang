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

class ByteCodeEmitter {

    private:
    std::map<std::string, std::shared_ptr<AST::Function>> functions;
    executor::Program program;

    struct Backpatch {
        size_t instruction_idx;
        std::string label;
    };
    std::vector<Backpatch> backpatches;
    std::vector<std::string> localNames; // Id is idx
    size_t num_params;  // Number of parameters for current function

    public:
    ByteCodeEmitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions);

    virtual void run();
    virtual std::string toString();

    executor::Program getProgram();

   protected:
    std::vector<executor::Instruction>& code() {
        return program.code;
    }

    void process(const std::shared_ptr<AST::Node>& node, bool hasConsumer);
    void loadIdentifier(const std::shared_ptr<AST::Identifier>& identifier);
    void storeLocalInto(const std::shared_ptr<AST::Node>& node);
    size_t allocStructs(const DataType::Struct& structType);
};

} // namespace emitter
