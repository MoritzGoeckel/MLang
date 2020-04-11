#pragma once

#include <memory>
#include <vector>

namespace AST {

enum class DataType { Int, Float, String, Bool, Void, Unknown, Conflict, None };

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
    DataType dataType = DataType::Unknown;

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

    virtual void infereDataType() = 0;
    virtual void hintDataType(DataType hint) {
        /*
         *  Should throw for most nodes.
         *  Is only valid for var / fn decl
         *  Nodes that want to support that
         *  have to override
         */
        throw;
    }
    virtual DataType
    getReturnType() {  // TODO this should be in its own visitor
        std::set<DataType> types;
        for (auto& child : getChildren()) {
            if (child) {
                child->infereDataType();
                types.insert(child->getReturnType());
            }
        }
        removeNoneDataType(types);

        if (types.size() > 1u) {
            return DataType::Conflict;
        } else if (types.empty()) {
            return DataType::None;
        } else {
            return *(types.begin());
        }
    }

    DataType getDataType() { return dataType; }
    std::string getDataTypeString() {
        if (dataType == DataType::Unknown) return {};
        return "[" + toString(dataType) + "]";
    }

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
                return "void";
            case DataType::Unknown:
                return "unknown";
            case DataType::Conflict:
                return "conflict";
            case DataType::None:
                return "None";
        }
    }

    static DataType toDataType(const std::string& str) {
        if (str == "int") return DataType::Int;
        if (str == "float") return DataType::Float;
        if (str == "string") return DataType::String;
        if (str == "bool") return DataType::Bool;
        if (str == "void") return DataType::Void;
        if (str == "conflict") return DataType::Conflict;
        if (str == "none") return DataType::None;
        return DataType::Unknown;
    }

    static void removeNoneDataType(std::set<DataType>& set,
                                   DataType type = DataType::None) {
        if (set.find(type) != set.end()) set.erase(type);
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
        stream << getDataTypeString() << "'" << id << "'";
    }

    virtual NodeType getType() { return NodeType::Identifier; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() { return {}; }

    virtual void infereDataType() { /* TODO */
    }

    virtual void hintDataType(DataType hint) { /* TODO */
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
        vec.resize(children.size());
        for (auto& c : children) {
            vec.emplace_back(c);
        }
        return vec;
    }

    void setChildren(std::vector<std::shared_ptr<Node>> nodes) {
        children = nodes;
    }

    virtual void infereDataType() override {
        std::set<DataType> types;
        for (auto& child : children) {
            child->infereDataType();
            types.insert(child->getReturnType());
        }
        removeNoneDataType(types);

        if (types.size() > 1u) {
            dataType = DataType::Conflict;
        } else if (types.empty()) {
            dataType = DataType::Void;
        } else {
            dataType = *(types.begin());
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
        vec.resize(arguments.size() + 1);
        vec.emplace_back(method);
        for (auto& a : arguments) {
            vec.emplace_back(a);
        }
        return vec;
    }

    virtual void infereDataType() override { /* TODO */
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

    virtual void infereDataType() override {
        if (expr) {
            expr->infereDataType();
            dataType = expr->getDataType();
        } else {
            dataType = DataType::Void;
        }
    }

    virtual DataType getReturnType() override {
        if (dataType == DataType::Unknown) infereDataType();
        return dataType;
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
            left->getDataType() != DataType::None) {
            // TODO: Maybe some implicit conversions
            dataType = DataType::Conflict;
        } else {
            left->hintDataType(dataType);
        }
    }
};

class Declvar : public Node {
   public:
    std::shared_ptr<Identifier> name;

   public:
    Declvar(std::shared_ptr<Node> name)
        : name(std::static_pointer_cast<Identifier>(name)) {}

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "declvar(";
        name->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Declvar; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {name};
    }

    virtual void infereDataType() override { dataType = DataType::None; }

    virtual void hintDataType(DataType hint) override {
        dataType = hint;
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
        vec.resize(parameters.size() + 1);
        vec.emplace_back(name);
        for (auto& p : parameters) {
            vec.emplace_back(p);
        }
        return vec;
    }

    virtual void infereDataType() override {
        // TODO parameters?
        dataType = DataType::None;
    }

    virtual void hintDataType(DataType hint) override {
        // TODO: Sure? What about its a function ...
        dataType = hint;
        name->hintDataType(hint);
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
        dataType = DataType::None;
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
        dataType = DataType::None;
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

    Literal(const std::string& value, std::string& dataType) : value(value) {
        this->dataType = toDataType(dataType);
    }

    virtual void toString(std::stringstream& stream) override {
        stream << Node::toString(dataType) << "('" << value << "')";
    }

    virtual NodeType getType() override { return NodeType::Literal; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }

    virtual void infereDataType() override { /* already set in constructor */
    }
};

}  // namespace AST
