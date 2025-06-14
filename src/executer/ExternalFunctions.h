#pragma once

#include "../error/Exceptions.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace ffi {

using word_t = unsigned long; // TODO?
using dword_t = unsigned long;
using qword_t = unsigned long long;

using float_t = float;
using double_t = double;

enum class Type {
    Word,
    DWord,
    QWord,
    Float,
    Double
};

struct ExternalFunction {
    std::string library;
    std::string name;
    void* functionPtr;
    std::vector<Type> parameters;
    Type returnType;
};

class ExternalFunctions {
   public:
    ExternalFunctions();

    size_t add(const std::string& library, const std::string& functionName);

    qword_t call(size_t id, const std::vector<void*>& args);

    ~ExternalFunctions();

   private:
    std::vector<ExternalFunction> functions;
    std::map<std::string, void*> libraries;
};

} // namespace ffi