#pragma once

#include <memory>
#include <vector>

namespace AST {

enum class DataType { Int, Float, String, Bool, Void, Unknown };

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
    DataType dataType = DataType::Void;

   public:
    std::string toString() {
        std::stringstream stream;
        toString(stream);
        return stream.str();
    };

    // virtual void emit() = 0;

    virtual void toString(std::stringstream& stream) = 0;
    virtual NodeType getType() = 0;
    virtual std::vector<std::shared_ptr<Node>> getChildren() = 0;

    DataType getDataType() { return dataType; }

    static std::string toString(DataType type) {
        switch (type) {
            case DataType::Int:
                return "int";
            case DataType::Float:
                return "float";
            case DataType::String:
                return "string";
            case DataType::Bool:
                return "bool";
            case DataType::Void:
                return "unknown";
            case DataType::Unknown:
                return "unknown";
        }
    }

    static DataType toDataType(const std::string& str) {
        if (str == "int") return DataType::Int;
        if (str == "float") return DataType::Float;
        if (str == "string") return DataType::String;
        if (str == "bool") return DataType::Bool;
        if (str == "void") return DataType::Void;
        return DataType::Unknown;
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
    virtual void toString(std::stringstream& stream) {
        stream << "'" << id << "'";
    }

    virtual NodeType getType() { return NodeType::Identifier; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() { return {}; }
};

class Block : public Node {
   private:
    std::vector<std::shared_ptr<Node>> children;

   public:
    Block(std::vector<std::shared_ptr<Node>> children) : children(children) {}

    virtual void toString(std::stringstream& stream) {
        stream << "{";
        print(stream, children);
        stream << "}";
    }

    virtual NodeType getType() { return NodeType::Block; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        std::vector<std::shared_ptr<Node>> vec;
        vec.resize(children.size());
        for (auto& c : children) {
            vec.emplace_back(c);
        }
        return vec;
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

    virtual void toString(std::stringstream& stream) {
        stream << "call(";
        method->toString(stream);
        if (!arguments.empty()) {
            stream << ", ";
            print(stream, arguments);
        }
        stream << ")";
    }

    virtual NodeType getType() { return NodeType::Call; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        std::vector<std::shared_ptr<Node>> vec;
        vec.resize(arguments.size() + 1);
        vec.emplace_back(method);
        for (auto& a : arguments) {
            vec.emplace_back(a);
        }
        return vec;
    }
};

class Ret : public Node {
   private:
    std::shared_ptr<Node> expr;

   public:
    Ret(std::shared_ptr<Node> expr) : expr(expr) {}
    Ret() {}

    virtual void toString(std::stringstream& stream) {
        stream << "ret(";
        if (expr) expr->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() { return NodeType::Ret; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() { return {expr}; }

    std::shared_ptr<Node> getExpr() { return expr; }
};

class Assign : public Node {
   private:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

   public:
    Assign(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
        : left(left), right(right) {}

    virtual void toString(std::stringstream& stream) {
        stream << "assign(";
        left->toString(stream);
        stream << ", ";
        right->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() { return NodeType::Assign; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        return {left, right};
    }

    std::shared_ptr<Node> getLeft() { return left; }
    std::shared_ptr<Node> getRight() { return right; }
};

class Declvar : public Node {
   public:
    std::shared_ptr<Identifier> name;

   public:
    Declvar(std::shared_ptr<Node> name)
        : name(std::static_pointer_cast<Identifier>(name)) {}

    virtual void toString(std::stringstream& stream) {
        stream << "declvar(";
        name->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() { return NodeType::Declvar; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() { return {name}; }
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

    virtual void toString(std::stringstream& stream) {
        stream << "declfn(";
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

    virtual NodeType getType() { return NodeType::Declfn; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        std::vector<std::shared_ptr<Node>> vec;
        vec.resize(parameters.size() + 1);
        vec.emplace_back(name);
        for (auto& p : parameters) {
            vec.emplace_back(p);
        }
        return vec;
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

    virtual void toString(std::stringstream& stream) {
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

    virtual NodeType getType() { return NodeType::If; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        return {condition, bodyPositive, bodyNegative};
    }
};

class While : public Node {
   private:
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> body;

   public:
    While(std::shared_ptr<Node> condition, std::shared_ptr<Node> body)
        : condition(condition), body(body) {}

    virtual void toString(std::stringstream& stream) {
        stream << "while(";
        condition->toString(stream);
        stream << ", ";
        body->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() { return NodeType::While; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() {
        return {condition, body};
    }
};

class Literal : public Node {
   private:
    std::string value;

   public:
    Literal(const std::string& value, DataType dataType) : value(value) {
        this->dataType = dataType;
    }

    Literal(const std::string& value, std::string& dataType) : value(value) {
        this->dataType = toDataType(dataType);
    }

    virtual void toString(std::stringstream& stream) {
        stream << Node::toString(dataType) << "('" << value << "')";
    }

    virtual NodeType getType() { return NodeType::Literal; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() { return {}; }
};

}  // namespace AST
