#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "../ast/DataType.h"
#include "../ast/Node.h"
#include "../error/Exceptions.h"
#include "../transformer/TreeWalker.h"

#include "Emitter.h"

namespace emitter {

class Python : public Emitter {
    private:
    std::stringstream code;
    int indent_level;
    std::map<std::string/* ast name */, std::string /* emitted name */> build_in_functions;
    std::set<std::string> used_build_in_functions;

    public:
    Python(const std::map<std::string, std::shared_ptr<AST::Function>> &functions);

    void instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast);

    virtual void run();
    virtual std::string toString();

   protected:
    virtual void process(const std::shared_ptr<AST::Node>& node);
    void process_inline(std::shared_ptr<AST::Node> node);
    void indent(); 
    void implement_build_in_functions();
    std::string formatName(const std::string &name);
};

} // namespace emitter