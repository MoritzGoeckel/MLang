#include "Emitter.h"

Emitter::Emitter(
    const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : functions(functions), lastId(0u) { }

Emitter::~Emitter() { }

void Emitter::run() {
    for (auto &fn : functions) {
        // fns[fn.first] = instantiateFn(fn.first, fn.second);
    }
}

void Emitter::print() { }

std::string Emitter::createUnique(const char *str) {
    return std::string(str) + "_" + std::to_string(lastId++);
}
std::string Emitter::createUnique(std::string str) {
    return createUnique(str.c_str());
}

void Emitter::instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast) { }

void Emitter::process(std::shared_ptr<AST::Node> node) { }

void Emitter::followChildren(std::shared_ptr<AST::Node> &node) {
    for (auto child : node->getChildren()) {
        if (!child) continue;
        process(child);
    }
}
