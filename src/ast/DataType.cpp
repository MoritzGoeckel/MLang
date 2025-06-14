#include "DataType.h"
#include "../error/Exceptions.h"

size_t DataType::Struct::getMemorySize() const {
    return fields.size();
}

DataType::DataType(DataType::Primitive primitive)
    : impl(Simple{primitive}) {}

DataType::DataType(const std::string& str) // TODO, maybe remove
    : impl(Simple{toPrimitive(str)}) {}

DataType::DataType(const std::vector<DataType>& params, DataType ret)
    : impl(Function{
          std::make_shared<const std::vector<DataType>>(params),
          std::make_shared<const DataType>(ret), false /*isExtern*/}) {}

DataType::DataType(const std::vector<DataType>& params, DataType ret, bool isExtern)
    : impl(Function{
          std::make_shared<const std::vector<DataType>>(params),
          std::make_shared<const DataType>(ret), 
          isExtern}) {}

DataType::DataType(DataType::Struct structType)
    : impl(structType) {}

DataType::DataType(const DataType& other)
    : impl(other.impl) {}

DataType::DataType() : impl(Simple{Primitive::Unknown}) {}

void DataType::operator=(const DataType& other) {
    impl = other.impl;
}

bool DataType::operator==(const DataType& other) const {
    if (impl.index() != other.impl.index()) {
        return false; // Different types
    }

    if (std::holds_alternative<Simple>(impl)) {
        return std::get<Simple>(impl).simple == std::get<Simple>(other.impl).simple;
    } else if (std::holds_alternative<Function>(impl)) {
        const auto& thisFunc = std::get<Function>(impl);
        const auto& otherFunc = std::get<Function>(other.impl);
        return *thisFunc.ret == *otherFunc.ret &&
               *thisFunc.params == *otherFunc.params &&
               thisFunc.isExtern == otherFunc.isExtern;
    } else if (std::holds_alternative<Struct>(impl)) {
        return std::get<Struct>(impl).name == std::get<Struct>(other.impl).name;
    }
    throwConstraintViolated("Unknown DataType variant in comparison");
}

bool DataType::operator==(Primitive primitive) const {
    if (!std::holds_alternative<Simple>(impl)) {
        return false; // Only simple types can be compared to primitives
    }
    const auto& simple = std::get<Simple>(impl).simple;
    return simple == primitive;
}

bool DataType::operator!=(const DataType& other) const {
    return !(other == *this);
}


bool DataType::operator!=(Primitive other) const {
    return !(*this == other);
}

bool DataType::operator<(const DataType& other) const {
    return other.getHashNum() < getHashNum();
}

size_t DataType::getMemorySize() const {
    if (std::holds_alternative<Simple>(impl)) {
        return 1ull;
    } else if (std::holds_alternative<Function>(impl)) {
        return 1ull;
    } else if (std::holds_alternative<Struct>(impl)) {
        return getStruct().getMemorySize();
    }
    throwConstraintViolated("Unknown DataType variant in getMemorySize");
    return 0; // Should never reach here
}

std::shared_ptr<const DataType> DataType::getReturn() const {
    if (!std::holds_alternative<Function>(impl)) {
        return nullptr; // Not a function type
    }
    const auto& func = std::get<Function>(impl);
    return func.ret;
}

std::shared_ptr<const std::vector<DataType>> DataType::getParams() const {
    if (!std::holds_alternative<Function>(impl)) {
        return nullptr; // Not a function type
    }
    const auto& func = std::get<Function>(impl);
    return func.params;
}
bool DataType::getIsPrimitive() const {
    return std::holds_alternative<Simple>(impl);
}

DataType::Primitive DataType::getPrimitive() const {
    if (!std::holds_alternative<Simple>(impl)) {
        return Primitive::None; // Not a simple type
    }
    return std::get<Simple>(impl).simple;
}

std::string DataType::toString() const {
    if (std::holds_alternative<Simple>(impl)) {
        const auto& simple = std::get<Simple>(impl).simple;
        return toString(simple);
    } else if (std::holds_alternative<Function>(impl)) {
        const auto& func = std::get<Function>(impl);
        const auto& params = func.params;
        const auto& ret = func.ret;
        std::stringstream stream;
        if(func.isExtern) {
            stream << "ext_";
        }
        stream << "[";
        if (params->empty()) stream << "]";
        for (auto it = params->begin(); it != params->end(); ++it) {
            stream << it->toString();
            if (std::next(it) != params->end()) {
                stream << ", ";
            }
        }
        if (params->size() >= 1u) stream << "]";
        stream << " -> " << ret->toString();
        return stream.str();
    } else if (std::holds_alternative<Struct>(impl)) {
        const auto& structType = std::get<Struct>(impl);
        return "struct " + structType.name;
    }
    throwConstraintViolated("Unknown DataType variant in toString");
    return "Unexpected DataType variant";
}

void DataType::removeNone(std::set<DataType::Primitive>& set,
                          DataType::Primitive type) {
    if (set.find(type) != set.end()) set.erase(type);
}

void DataType::removeNone(std::set<DataType>& set, DataType::Primitive type) {
    if (set.find(type) != set.end()) set.erase(type);
}

std::string DataType::toString(DataType::Primitive type) {
    switch (type) {
        case DataType::Primitive::Int:
            return "int";
        case DataType::Primitive::Float:
            return "float";
        case DataType::Primitive::String:
            return "string";
        case DataType::Primitive::Bool:
            return "bool";
        case DataType::Primitive::Void:
            return "void";
        case DataType::Primitive::Struct:
            return "struct";
        case DataType::Primitive::Unknown:
            return "unknown";
        case DataType::Primitive::Conflict:
            return "conflict";
        case DataType::Primitive::None:
            return "None";
    }
    throwConstraintViolated("Unknown DataType::Primitive in toString");
    return "Unknown DataType::Primitive"; // Should never reach here
}

DataType::Primitive DataType::toPrimitive(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (lowerStr == "int") return DataType::Primitive::Int;
    if (lowerStr == "float") return DataType::Primitive::Float;
    if (lowerStr == "string") return DataType::Primitive::String;
    if (lowerStr == "bool") return DataType::Primitive::Bool;
    if (lowerStr == "void") return DataType::Primitive::Void;
    if (lowerStr == "conflict") return DataType::Primitive::Conflict;
    if (lowerStr == "none") return DataType::Primitive::None;
    return DataType::Primitive::Unknown;
}

size_t DataType::getHashNum() const {
    if (std::holds_alternative<Simple>(impl)) {
        const auto& simple = std::get<Simple>(impl).simple;
        return static_cast<size_t>(simple);
    } else if (std::holds_alternative<Function>(impl)) {
        const auto& function = std::get<Function>(impl);
        size_t sum;
        for (const auto& p : *function.params) sum += p.getHashNum();
        sum += function.ret->getHashNum();
        return sum;
    } else if (std::holds_alternative<Struct>(impl)) {
        const auto& structType = std::get<Struct>(impl);
        return std::hash<std::string>()(structType.name);
    }
    throwConstraintViolated("Unknown DataType variant in getHashNum");
}
