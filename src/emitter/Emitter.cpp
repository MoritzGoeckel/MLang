#include "Emitter.h"

namespace emitter {

Emitter::Emitter(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : functions(functions), lastId(0u) { }

std::string Emitter::createUnique(const char *str) {
    return std::string(str) + "_" + std::to_string(lastId++);
}

std::string Emitter::createUnique(std::string str) {
    return createUnique(str.c_str());
}

void Emitter::print() {
    std::cout << "Generated code" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << toString() << std::endl;
    std::cout << "------------------------" << std::endl;
}

void Emitter::followChildren(const std::shared_ptr<AST::Node>& node) {
    for (auto child : node->getChildren()) {
        if (!child) continue;
        process(child);
    }
}

} // namespace emitter