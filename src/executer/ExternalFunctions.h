#pragma once

#include "../error/Exceptions.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace ffi {

using byte_t = unsigned char;
using word_t = unsigned short;
using dword_t = unsigned int;
using qword_t = unsigned long long;
using xmmword_t = unsigned char[16]; // Probably we don't need this, we might for floats

using ptr = void*;
static_assert(sizeof(ptr) == 8);

static_assert(sizeof(word_t) == 2);
static_assert(sizeof(dword_t) == 4);
static_assert(sizeof(qword_t) == 8);
static_assert(sizeof(xmmword_t) == 16);

namespace arg_types{
    using type = qword_t;
    constexpr type None = 0;
    constexpr type Word = 1;
    constexpr type DWord = 2;
    constexpr type QWord = 3;
    constexpr type Float = 4;
    // constexpr byte_t XMMWord = 4; // Not used yet
}

struct arg_t {
    arg_types::type type; // This cannot be smaller than qword_t
    qword_t value; // Use the largest type
};

static_assert(sizeof(arg_t) == sizeof(qword_t) + sizeof(arg_types::type));

class Arguments {
    public:
    Arguments();

    void addWord(word_t value);
    void addDWord(dword_t value);
    void addQWord(qword_t value);
    
    void clear();

    static constexpr size_t capacity = 6;
    
    size_t getSize() const { return size; }
    const arg_t* getBuffer() const { return buffer; }

    private:
    size_t size;
    arg_t buffer[capacity];
};

struct ExternalFunction {
    std::string library;
    std::string name;
    void* functionPtr;
};

class ExternalFunctions {
   public:
    ExternalFunctions();

    size_t add(const std::string& library, const std::string& functionName);

    qword_t call(size_t id, const Arguments& args);

    ~ExternalFunctions();

   private:
    std::vector<ExternalFunction> functions;
    std::map<std::string, void*> libraries;
};

} // namespace ffi