#include "DataType.h"

DataType::DataType(DataType::Primitive primitive)
    : isSimple(true), simple(primitive) {}

DataType::DataType(const std::string& str)
    : isSimple(true), simple(toPrimitive(str)) {}

// Complex type constructor
DataType::DataType(std::vector<DataType> params, DataType ret)
    : isSimple(false),
      params(std::make_shared<const std::vector<DataType>>(params)),
      ret(std::make_shared<const DataType>(ret)) {}

DataType::DataType(const DataType& other)
    : isSimple(other.isSimple),
      simple(other.simple),
      params(other.params),
      ret(other.ret) {}

DataType::DataType() : isSimple(true), simple(DataType::Primitive::Unknown) {}

void DataType::operator=(const DataType& other) {
    isSimple = other.isSimple;
    simple = other.simple;
    params = other.params;
    ret = other.ret;
}

bool DataType::operator==(const DataType& other) const {
    if (other.isSimple != isSimple) return false;
    if (isSimple) {
        return other.simple == simple;
    } else {
        return *(other.params) == *params && *(other.ret) == *ret;
    }
}

bool DataType::operator==(Primitive primitive) const {
    if (isSimple) {
        return simple == primitive;
    } else {
        return false; // Complex types cannot be equal to a primitive
    }
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

std::shared_ptr<const DataType> DataType::getReturn() const { return ret; }
std::shared_ptr<const std::vector<DataType>> DataType::getParams() const {
    return params;
}
bool DataType::getIsPrimitive() const { return isSimple; }
DataType::Primitive DataType::getPrimitive() const {
    if (!isSimple) throw "Accessing simple type of complex type";
    return simple;
}

std::string DataType::toString() const {
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
        case DataType::Primitive::Unknown:
            return "unknown";
        case DataType::Primitive::Conflict:
            return "conflict";
        case DataType::Primitive::None:
            return "None";
    }
}

DataType::Primitive DataType::toPrimitive(const std::string& str) {
    if (str == "int") return DataType::Primitive::Int;
    if (str == "float") return DataType::Primitive::Float;
    if (str == "string") return DataType::Primitive::String;
    if (str == "bool") return DataType::Primitive::Bool;
    if (str == "void") return DataType::Primitive::Void;
    if (str == "conflict") return DataType::Primitive::Conflict;
    if (str == "none") return DataType::Primitive::None;
    return DataType::Primitive::Unknown;
}

size_t DataType::getHashNum() const {
    if (isSimple) {
        return static_cast<int>(simple);
    } else {
        size_t sum;
        for (const auto& p : *params) sum += p.getHashNum();
        sum += ret->getHashNum();
        return sum;
    }
}
