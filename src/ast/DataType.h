#pragma once

#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

class DataType {
   public:
    enum class Primitive {
        Int,
        Float,
        String,
        Bool,
        Void,
        Unknown,
        Conflict,
        None
    };

   private:
    bool isSimple;

    // Simple:
    // Just a Primitive
    Primitive simple;

    // Complex:
    // Types[] -> Type
    std::shared_ptr<const std::vector<DataType>> params;
    std::shared_ptr<const DataType> ret;

   public:
    // Simple type constructors
    DataType(Primitive primitive);
    DataType(const std::string& str);

    // Complex type constructor
    DataType(std::vector<DataType> params, DataType ret);

    DataType(const DataType& other);

    DataType();

    void operator=(const DataType& other);

    bool operator==(const DataType& other) const;

    bool operator!=(const DataType& other) const;

    bool operator<(const DataType& other) const;

    std::shared_ptr<const DataType> getReturn() const;
    std::shared_ptr<const std::vector<DataType>> getParams() const;
    bool getIsPrimitive() const;
    Primitive getPrimitive() const;

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
