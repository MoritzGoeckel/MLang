#pragma once

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
    DataType(Primitive primitive) : isSimple(true), simple(primitive) {}
    DataType(const std::string& str)
        : isSimple(true), simple(toPrimitive(str)) {}

    // Complex type constructor
    DataType(std::vector<DataType> params, DataType ret)
        : isSimple(false),
          params(std::make_shared<const std::vector<DataType>>(params)),
          ret(std::make_shared<const DataType>(ret)) {}

    DataType(const DataType& other)
        : isSimple(other.isSimple),
          simple(other.simple),
          params(other.params),
          ret(other.ret) {}

    DataType() : isSimple(true), simple(Primitive::Unknown) {}

    void operator=(const DataType& other) {
        isSimple = other.isSimple;
        simple = other.simple;
        params = other.params;
        ret = other.ret;
    }

    bool operator==(const DataType& other) const {
        if (other.isSimple != isSimple) return false;
        if (isSimple) {
            return other.simple == simple;
        } else {
            return *(other.params) == *params && *(other.ret) == *ret;
        }
    }

    bool operator!=(const DataType& other) const { return !(other == *this); }

    bool operator<(const DataType& other) const {
        return other.getHashNum() < getHashNum();
    }

    std::shared_ptr<const DataType> getReturn() { return ret; }

    std::string toString() const {
        if (isSimple) {
            return toString(simple);
        } else {
            // Complex
            std::stringstream stream;
            if (params->size() > 1u) stream << "[";
            if (params->empty()) stream << "[]";
            for (auto it = params->begin(); it != params->end(); ++it) {
                stream << it->toString();
                if (std::next(it) != params->end()) {
                    stream << ", ";
                }
            }
            if (params->size() > 1u) stream << "]";
            stream << " -> " << ret->toString();
            return stream.str();
        }
    }

   public:
    static void removeNone(std::set<Primitive>& set,
                           Primitive type = Primitive::None) {
        if (set.find(type) != set.end()) set.erase(type);
    }

    static void removeNone(std::set<DataType>& set,
                           Primitive type = Primitive::None) {
        if (set.find(type) != set.end()) set.erase(type);
    }

    template <typename T>
    static bool containsUnknown(const T& container) {
        return std::any_of(container.begin(), container.end(),
                           [](const DataType& t) {
                               return t == DataType::Primitive::Unknown;
                           });
    }

    static std::string toString(Primitive type) {
        switch (type) {
            case Primitive::Int:
                return "int";
            case Primitive::Float:
                return "float";
            case Primitive::String:
                return "string";
            case Primitive::Bool:
                return "bool";
            case Primitive::Void:
                return "void";
            case Primitive::Unknown:
                return "unknown";
            case Primitive::Conflict:
                return "conflict";
            case Primitive::None:
                return "None";
        }
    }

    static Primitive toPrimitive(const std::string& str) {
        if (str == "int") return Primitive::Int;
        if (str == "float") return Primitive::Float;
        if (str == "string") return Primitive::String;
        if (str == "bool") return Primitive::Bool;
        if (str == "void") return Primitive::Void;
        if (str == "conflict") return Primitive::Conflict;
        if (str == "none") return Primitive::None;
        return Primitive::Unknown;
    }

   private:
    size_t getHashNum() const {
        if (isSimple) {
            return static_cast<int>(simple);
        } else {
            size_t sum;
            for (const auto& p : *params) sum += p.getHashNum();
            sum += ret->getHashNum();
            return sum;
        }
    }
};
