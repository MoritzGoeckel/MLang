#pragma once

#include <memory>
#include <vector>

#include "DataType.h"

namespace AST {

enum class NodeType {
    Identifier,
    Block,
    Call,
    Ret,
    Assign,
    Declvar,
    Declfn,
    If,
    While,
    Literal
};

class Node {
   protected:
    DataType dataType;

   public:
    Node() : dataType(DataType::Primitive::Unknown){};

    std::string toString() {
        std::stringstream stream;
        toString(stream);
        return stream.str();
    };

    // virtual void emit() = 0;

    virtual void toString(std::stringstream& stream) = 0;
    virtual NodeType getType() = 0;
    virtual std::vector<std::shared_ptr<Node>> getChildren() = 0;

    virtual void infereDataType() = 0;
    virtual void hintDataType(DataType hint) {
        /*
         *  Should throw for most nodes.
         *  Is only valid for var / fn decl
         *  Nodes that want to support that
         *  have to override
         */
        throw;  // TODO: Emit messages
    }
    virtual DataType
    getReturnType() {  // TODO this should be in its own visitor, same for the
                       // "infereType" functions
        // TODO: Cache?
        std::set<DataType> types;
        for (auto& child : getChildren()) {
            if (child) {
                child->infereDataType();
                types.insert(child->getReturnType());
            }
        }
        DataType::removeNone(types);

        if (DataType::containsUnknown(types))
            return DataType::Primitive::Unknown;

        if (types.size() > 1u) {
            return DataType::Primitive::Conflict;
            // TODO: Message out
        } else if (types.empty()) {
            return DataType::Primitive::None;
        } else {
            return *(types.begin());
        }
    }

    virtual DataType getDataType() { return dataType; }

    std::string getDataTypeString() {
        auto type = getDataType();
        if (type == DataType::Primitive::Unknown) return "[U]";
        if (type == DataType::Primitive::None) return {};
        return "[" + type.toString() + "]";
    }

    template <typename T>
    void print(std::stringstream& stream, const T& vec) {
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            (*it)->toString(stream);
            if (std::next(it) != vec.end()) {
                stream << ", ";
            }
        }
    }
};

class Identifier : public Node {
   private:
    std::string id;

   public:
    Identifier(const std::string& id) : id(id) {}

    std::string getName() { return id; }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "'" << id << "'";
    }

    virtual NodeType getType() override { return NodeType::Identifier; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }

    virtual void infereDataType() override { /* TODO */
    }

    virtual void hintDataType(DataType hint) override {
        if (dataType == DataType::Primitive::Unknown || dataType == hint)
            dataType = hint;
        else {
            // TODO: Emit messages
            std::cout << "-- Conflicting type " << dataType.toString() << " / "
                      << hint.toString() << std::endl;

            dataType = DataType::Primitive::Conflict;
        }
    }
};

class Block : public Node {
   private:
    std::vector<std::shared_ptr<Node>> children;

   public:
    Block(std::vector<std::shared_ptr<Node>> children) : children(children) {}

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "{";
        print(stream, children);
        stream << "}";
    }

    virtual NodeType getType() override { return NodeType::Block; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(children.size());
        for (auto& c : children) {
            vec.emplace_back(c);
        }
        return vec;
    }

    void setChildren(std::vector<std::shared_ptr<Node>> nodes) {
        children = nodes;
    }

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual DataType getReturnType() override {
        std::set<DataType> types;
        for (auto& child : children) {
            child->infereDataType();
            types.insert(child->getReturnType());
        }
        DataType::removeNone(types);

        if (DataType::containsUnknown(types))
            return DataType::Primitive::Unknown;

        if (types.size() > 1u) {
            // TODO: Message out
            return DataType::Primitive::Conflict;
        } else if (types.empty()) {
            return DataType::Primitive::Void;
        } else {
            return *(types.begin());
        }
    }

    virtual void infereDataType() override {
        for (auto& child : children) {
            child->infereDataType();
        }
    }
};

class Call : public Node {
   private:
    std::shared_ptr<Identifier> method;
    std::vector<std::shared_ptr<Node>> arguments;

   public:
    Call(std::shared_ptr<Node> method,
         std::vector<std::shared_ptr<Node>> arguments)
        : method(std::static_pointer_cast<Identifier>(method)),
          arguments(arguments) {}

    std::vector<std::shared_ptr<Node>>& getArguments() { return arguments; }
    std::shared_ptr<Identifier>& getIdentifier() { return method; }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "call(";
        method->toString(stream);
        if (!arguments.empty()) {
            stream << ", ";
            print(stream, arguments);
        }
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Call; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(arguments.size() + 1);
        vec.emplace_back(method);
        for (auto& a : arguments) {
            vec.emplace_back(a);
        }
        return vec;
    }

    virtual void infereDataType() override {
        // Not possible at this point. Is done in a tree walker
    }

    virtual DataType getDataType() override {
        if (getIdentifier()->getDataType() != DataType::Primitive::Unknown)
            return *(getIdentifier()->getDataType().getReturn());
        else
            return DataType::Primitive::Unknown;
    }

    virtual void hintDataType(DataType hint) override {
        getIdentifier()->hintDataType(hint);
    }
};

class Ret : public Node {
   private:
    std::shared_ptr<Node> expr;

   public:
    Ret(std::shared_ptr<Node> expr) : expr(expr) {}
    Ret() {}

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "ret(";
        if (expr) expr->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Ret; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {expr};
    }

    std::shared_ptr<Node> getExpr() { return expr; }

    virtual void infereDataType() override {}
    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual DataType getReturnType() override {
        if (expr) {
            expr->infereDataType();
            return expr->getDataType();
        } else {
            return DataType::Primitive::Void;
        }
    }
};

class Assign : public Node {
   private:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

   public:
    Assign(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : left(left), right(right) {}

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "assign(";
        left->toString(stream);
        stream << ", ";
        right->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Assign; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {left, right};
    }

    std::shared_ptr<Node> getLeft() { return left; }
    std::shared_ptr<Node> getRight() { return right; }
    void setRight(std::shared_ptr<Node> node) { right = node; }

    virtual void infereDataType() override {
        right->infereDataType();
        dataType = right->getDataType();
        left->infereDataType();
        if (left->getDataType() != dataType &&
            left->getDataType() != DataType::Primitive::None) {
            // TODO: Maybe some implicit conversions

            if (left->getDataType() == DataType::Primitive::Unknown ||
                right->getDataType() == DataType::Primitive::Unknown)
                return;

            // TODO: Message out
            dataType = DataType::Primitive::Conflict;
        } else {
            left->hintDataType(dataType);
        }
    }
};

class Declvar : public Node {
   public:  // TODO: Private
    std::shared_ptr<Identifier> name;

   public:
    Declvar(std::shared_ptr<Node> name)
        : name(std::static_pointer_cast<Identifier>(name)) {}

    std::shared_ptr<Identifier> getIdentifier() { return name; }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "declvar(";
        name->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Declvar; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {name};
    }

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual void infereDataType() override {
        // dataType = name->getDataType();
    }

    virtual void hintDataType(DataType hint) override {
        // dataType = hint;
        name->hintDataType(hint);
    }
};

class Declfn : public Node {
   private:
    std::shared_ptr<Identifier> name;
    std::vector<std::shared_ptr<Identifier>> parameters;

   public:
    Declfn(std::shared_ptr<Node> name,
           std::vector<std::shared_ptr<Node>> parameters)
        : name(std::static_pointer_cast<Identifier>(name)), parameters() {
        for (auto& p : parameters) {
            this->parameters.push_back(std::static_pointer_cast<Identifier>(p));
        }
    }

    std::vector<std::shared_ptr<Identifier>>& getParameters() {
        return parameters;
    }

    std::shared_ptr<Identifier>& getIdentifier() { return name; }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "declfn(";
        name->toString(stream);
        stream << ", params(";
        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            (*it)->toString(stream);
            if (std::next(it) != parameters.end()) {
                stream << ", ";
            }
        }
        stream << "))";
    }

    virtual NodeType getType() override { return NodeType::Declfn; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(parameters.size() + 1);
        vec.emplace_back(name);
        for (auto& p : parameters) {
            vec.emplace_back(p);
        }
        return vec;
    }

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual void infereDataType() override {
        // TODO parameters?
        // dataType = DataType::Primitive::None;
    }

    virtual void hintDataType(DataType hint) override {
        // TODO: Sure? What about its a function ...
        // dataType = hint;
        if (hint != DataType::Primitive::None) name->hintDataType(hint);
    }
};

class If : public Node {
   private:
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> bodyPositive;
    std::shared_ptr<Node> bodyNegative;

   public:
    If(std::shared_ptr<Node> condition, std::shared_ptr<Node> bodyPositive,
       std::shared_ptr<Node> bodyNegative)
        : condition(condition),
          bodyPositive(bodyPositive),
          bodyNegative(bodyNegative) {}

    virtual void toString(std::stringstream& stream) override {
        stream << "if(";
        condition->toString(stream);

        stream << ", ";
        bodyPositive->toString(stream);

        if (bodyNegative) {
            stream << ", ";
            bodyNegative->toString(stream);
        }
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::If; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {condition, bodyPositive, bodyNegative};
    }

    virtual void infereDataType() override {
        dataType = DataType::Primitive::None;
        condition->infereDataType();
        bodyPositive->infereDataType();
        if (bodyNegative) bodyNegative->infereDataType();
    }
};

class While : public Node {
   private:
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> body;

   public:
    While(std::shared_ptr<Node> condition, std::shared_ptr<Node> body)
        : condition(condition), body(body) {}

    virtual void toString(std::stringstream& stream) override {
        stream << "while(";
        condition->toString(stream);
        stream << ", ";
        body->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::While; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {condition, body};
    }

    virtual void infereDataType() override {
        dataType = DataType::Primitive::None;
        condition->infereDataType();
        body->infereDataType();
    }
};

class Literal : public Node {
   private:
    std::string value;

   public:
    Literal(const std::string& value, DataType dataType) : value(value) {
        this->dataType = dataType;
    }

    Literal(const std::string& value, std::string& dataTypeStr) : value(value) {
        this->dataType = dataTypeStr;
    }

    virtual void toString(std::stringstream& stream) override {
        stream << dataType.toString() << "('" << value << "')";
    }

    virtual NodeType getType() override { return NodeType::Literal; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }

    virtual void infereDataType() override { /* already set in constructor */
    }
};

}  // namespace AST
