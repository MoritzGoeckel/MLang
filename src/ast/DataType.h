#pragma once

#include <functional>
#include <memory>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <variant>
#include "../error/Exceptions.h"

struct StructMember;

class DataType {
   public:
    enum class Primitive {
        Int,
        Float,
        String,
        Bool,
        Void,
        Struct,
        Unknown,
        Conflict,
        None
    };

    struct Struct{
        std::string name;
        std::map<std::string, StructMember> fields;
        size_t getMemorySize() const;
    };

   private:
    struct Simple {
        Primitive simple;
    };

    struct Function{
        // Types[] -> Type
        std::shared_ptr<const std::vector<DataType>> params;
        std::shared_ptr<const DataType> ret;
        bool isExtern;
    };

    std::variant<Simple, Function, Struct> impl;

   public:
    // Simple type constructors
    DataType(Primitive primitive);
    DataType(const std::string& str);

    // Function type constructor
    DataType(const std::vector<DataType>& params, DataType ret);
    DataType(const std::vector<DataType>& params, DataType ret, bool isExtern);

    // Struct
    DataType(Struct structType);

    DataType(const DataType& other);

    DataType();

    void operator=(const DataType& other);

    bool operator==(const DataType& other) const;
    bool operator==(Primitive primitive) const;

    bool operator!=(const DataType& other) const;
    bool operator!=(Primitive primitive) const;

    bool operator<(const DataType& other) const;

    size_t getMemorySize() const;

    std::shared_ptr<const DataType> getReturn() const;
    std::shared_ptr<const std::vector<DataType>> getParams() const;
    bool getIsPrimitive() const;
    Primitive getPrimitive() const;

    bool isStruct() const {
        return std::holds_alternative<Struct>(impl);
    }

    const Struct& getStruct() const {
        if (!isStruct()) {
            throwConstraintViolated("DataType is not a struct");
        }
        return std::get<Struct>(impl);
    }

    Struct& getStruct() {
        if (!isStruct()) {
            throwConstraintViolated("DataType is not a struct");
        }
        return std::get<Struct>(impl);
    }

    bool isFunction() const {
        return std::holds_alternative<Function>(impl);
    }

    const Function& getFunction() const {
        if (!isFunction()) {
            throwConstraintViolated("DataType is not a function");
        }
        return std::get<Function>(impl);
    }

    bool isPrimitive() const {
        return std::holds_alternative<Simple>(impl);
    }

    std::string toString() const;

   public:
    static void removeNone(std::set<Primitive>& set,
                           Primitive type = Primitive::None);

    static void removeNone(std::set<DataType>& set,
                           Primitive type = Primitive::None);

    template <typename T>
    static bool containsUnknown(const T& container) {
        return std::any_of(container.begin(), container.end(),
                           [](const DataType& t) {
                               return t == DataType::Primitive::Unknown;
                           });
    }

    static std::string toString(Primitive type);

    static Primitive toPrimitive(const std::string& str);

   private:
    size_t getHashNum() const;
};


const size_t INVALID_OFFSET = std::numeric_limits<size_t>::max();

struct StructMember {
    DataType type;
    size_t offset; // Offset in bytes from the start of the struct
};