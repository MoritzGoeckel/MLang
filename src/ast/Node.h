#pragma once
#include <memory>
#include <vector>

namespace AST {

enum Type { Int, Float, String, Bool, Unknown };

class Node {
   public:
    std::string toString() {
        std::stringstream stream;
        toString(stream);
        return stream.str();
    };
    // virtual void emit() = 0;

    virtual void toString(std::stringstream& stream) = 0;

    static std::string toString(Type type) {
        switch (type) {
            case Int:
                return "int";
            case Float:
                return "float";
            case String:
                return "string";
            case Bool:
                return "bool";
            case Unknown:
                return "unknown";
        }
    }

    static Type toType(const std::string& str) {
        if (str == "int") return Int;
        if (str == "float") return Float;
        if (str == "string") return String;
        if (str == "bool") return Bool;
        return Unknown;
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
    virtual void toString(std::stringstream& stream) { stream << id; }
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

   public:
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
        stream << "call('";
        method->toString(stream);
        if (!arguments.empty()) {
            stream << "', ";
            print(stream, arguments);
        }
        stream << ")";
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
};

class Literal : public Node {
   private:
    std::string value;
    Type type;

   public:
    Literal(const std::string& value, Type type) : value(value), type(type) {}
    Literal(const std::string& value, std::string& type)
        : value(value), type(toType(type)) {}

    virtual void toString(std::stringstream& stream) {
        stream << Node::toString(type) << "(" << value << ")";
    }
};

}  // namespace AST
