#pragma once

#include <memory>
#include <vector>

#include "../error/Exceptions.h"
#include "../parser/SourcePosition.h"
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
    Literal,
    Function,
    FnPtr,
    DeclStruct,
    StructAccess,
    ExternFn
};

class Node {
   protected:
    using AddMsgFn = std::function<void(const std::string&)>;
    DataType dataType;
    SourcePosition itsSourcePosition;

   public:
    Node() : dataType(DataType::Primitive::Unknown), itsSourcePosition(){};

    Node(const SourcePosition& thePosition)
        : dataType(DataType::Primitive::Unknown),
          itsSourcePosition(thePosition){};

    std::string toString() {
        std::stringstream stream;
        toString(stream);
        return stream.str();
    };

    const SourcePosition& getPosition() { return itsSourcePosition; }

    virtual void toString(std::stringstream& stream) = 0;

    virtual NodeType getType() = 0;
    virtual std::vector<std::shared_ptr<Node>> getChildren() = 0;

    virtual DataType getReturnType(AddMsgFn addMessage) {
        std::set<DataType> types;
        for (auto& child : getChildren()) {
            if (child) {
                types.insert(child->getReturnType(addMessage));
            }
        }
        DataType::removeNone(types);

        if (DataType::containsUnknown(types))
            return DataType::Primitive::Unknown;

        if (types.size() > 1u) {
            return DataType::Primitive::Conflict;
            std::string msg("Conflicting return types: ");
            for (auto& t : types) msg += t.toString() + " ";
            throwConstraintViolated(msg.c_str());  // TODO
            addMessage(msg);
        } else if (types.empty()) {
            return DataType::Primitive::None;
        }
        return *(types.begin());
    }

    virtual DataType getDataType() { return dataType; }

    std::string getDataTypeString() {
        auto type = getDataType();
        if (type == DataType::Primitive::Unknown) return "[U]";
        if (type == DataType::Primitive::None) return "[N]";
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
    std::string typeAnnotation;

   public:
    Identifier(const std::string& id) : Node(), id(id), typeAnnotation{} {}
    Identifier(const std::string& id, const SourcePosition& thePosition)
        : Node(thePosition), id(id), typeAnnotation{} {}

    std::string getName() { return id; }

    void setTypeAnnotation(const std::string& type) {
        typeAnnotation = type;
    }

    bool hasTypeAnnotation() const {
        return !typeAnnotation.empty();
    }

    const std::string& getTypeAnnotation() const {
        return typeAnnotation;
    }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "'" << id << "'";
    }

    virtual NodeType getType() override { return NodeType::Identifier; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }

    void setDataType(DataType type, AddMsgFn addMessage) {
        if (dataType == DataType::Primitive::Unknown || dataType == type)
            dataType = type;
        else {
            dataType = DataType::Primitive::Conflict;
            addMessage("Conflicting types: set " + dataType.toString() +
                       " to " + type.toString());
        }
    }
};

class Block : public Node {
   private:
    std::vector<std::shared_ptr<Node>> children;

   public:
    Block(std::vector<std::shared_ptr<Node>> children) : children(children) {}
    Block(std::vector<std::shared_ptr<Node>> children,
          const SourcePosition& thePosition)
        : Node(thePosition), children(children) {}

    Block(const SourcePosition& thePosition) : Node(thePosition), children() {}

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

    void addChild(std::shared_ptr<Node> node) {
        children.emplace_back(node);
    }

    void setChildren(std::vector<std::shared_ptr<Node>> nodes) {
        children = nodes;
    }

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual DataType getReturnType(AddMsgFn addMessage) override {
        std::set<DataType> types;
        for (auto& child : children) {
            types.insert(child->getReturnType(addMessage));
        }
        DataType::removeNone(types);

        if (DataType::containsUnknown(types))
            return DataType::Primitive::Unknown;

        if (types.size() > 1u) {
            std::string msg("Conflicting return types: ");
            for (auto& t : types) msg += t.toString() + " ";
            throwConstraintViolated(msg.c_str());  // TODO
            addMessage(msg);
            return DataType::Primitive::Conflict;
        } else if (types.empty()) {
            return DataType::Primitive::None;
        } else {
            return *(types.begin());
        }
    }
};

class Call : public Node {
   private:
    std::shared_ptr<Identifier> method;
    std::vector<std::shared_ptr<Node>> arguments;

   public:
    Call(std::shared_ptr<Identifier> method,
         std::vector<std::shared_ptr<Node>> arguments)
        : method(method), arguments(arguments) {}

    Call(std::shared_ptr<Identifier> method,
         std::vector<std::shared_ptr<Node>> arguments,
         const SourcePosition& thePosition)
        : Node(thePosition), method(method), arguments(arguments) {}

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

    virtual DataType getDataType() override {
        ASSURE_NOT_NULL(getIdentifier());
        if (getIdentifier()->getDataType() != DataType::Primitive::Unknown){
            auto ret = getIdentifier()->getDataType().getReturn();
            if(!ret) {
                return DataType::Primitive::Unknown;
            }
            return *ret;
        }
        else
            return DataType::Primitive::Unknown;
    }
};

class Ret : public Node {
   private:
    std::shared_ptr<Node> expr;

   public:
    Ret(std::shared_ptr<Node> expr) : expr(expr) {}
    Ret(std::shared_ptr<Node> expr, const SourcePosition& thePosition)
        : Node(thePosition), expr(expr) {}

    Ret() {}
    Ret(const SourcePosition& thePosition) : Node(thePosition), expr() {}

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

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }

    virtual DataType getReturnType(AddMsgFn addMessage) override {
        if (expr) {
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

    Assign(std::shared_ptr<Node> left, std::shared_ptr<Node> right,
           const SourcePosition& thePosition)
        : Node(thePosition), left(left), right(right) {}

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
    void setLeft(std::shared_ptr<Node> node) { left = node; }

    virtual DataType getDataType() override { return right->getDataType(); }
};

class Declvar : public Node {
   private:
    std::shared_ptr<Identifier> name;
    std::string typeAnnotation;

   public:
    Declvar(std::shared_ptr<Identifier> name) : name(name), typeAnnotation{} {}

    Declvar(std::shared_ptr<Identifier> name, const SourcePosition& thePosition)
        : Node(thePosition), name(name), typeAnnotation{} {}

    std::shared_ptr<Identifier> getIdentifier() { return name; }

    void setTypeAnnotation(const std::string& type) {
        typeAnnotation = type;
    }

    const std::string& getTypeAnnotation() const {
        return typeAnnotation;
    }

    bool hasTypeAnnotation() const {
        return !typeAnnotation.empty();
    }

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
};

class Declfn : public Node {
   private:
    std::shared_ptr<Identifier> name;
    std::vector<std::shared_ptr<Identifier>> parameters;

   public:
    Declfn(std::shared_ptr<Identifier> name,
           std::vector<std::shared_ptr<Identifier>> parameters)
        : name(name), parameters() {
        for (auto& p : parameters) {
            this->parameters.push_back(p);
        }
    }

    Declfn(std::shared_ptr<Identifier> name,
           std::vector<std::shared_ptr<Identifier>> parameters,
           const SourcePosition& thePosition)
        : Node(thePosition), name(name), parameters() {
        for (auto& p : parameters) {
            this->parameters.push_back(p);
        }
    }

    Declfn(const std::string& name)
        : Node(), name(std::make_shared<Identifier>(name)), parameters() {}

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
};


class ExternFn : public Node {
   private:
    std::string name;
    std::string library;
    std::vector<std::shared_ptr<Identifier>> parameters;
    std::string typeAnnotation;

   public:
    ExternFn(std::shared_ptr<Identifier> name,
           std::shared_ptr<Identifier> library,
           std::vector<std::shared_ptr<Identifier>> parameters,
           const SourcePosition& thePosition)
        : Node(thePosition), name(name->getName()), library(library->getName()), parameters(), typeAnnotation{} {
        for (auto& p : parameters) {
            this->parameters.push_back(p);
        }
    }

    std::vector<std::shared_ptr<Identifier>>& getParameters() {
        return parameters;
    }

    const std::string& getName() { return name; }
    const std::string& getLibrary() { return library; }

    void setTypeAnnotation(const std::string& type) {
        typeAnnotation = type;
    }

    bool hasTypeAnnotation() const {
        return !typeAnnotation.empty();
    }

    const std::string& getTypeAnnotation() const {
        return typeAnnotation;
    }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "externfn(";
        stream << getLibrary() << "::" << getName();
        stream << ", params(";
        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            (*it)->toString(stream);
            if (std::next(it) != parameters.end()) {
                stream << ", ";
            }
        }
        stream << "))";
    }

    virtual NodeType getType() override { return NodeType::ExternFn; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(parameters.size() + 1);
        for (auto& p : parameters) {
            vec.emplace_back(p);
        }
        return vec;
    }

    virtual DataType getDataType() override {
        return dataType;
    }

    void setDataType(DataType type, AddMsgFn addMessage) {
        if (dataType == DataType::Primitive::Unknown || dataType == type)
            dataType = type;
        else {
            dataType = DataType::Primitive::Conflict;
            addMessage("Conflicting types: set " + dataType.toString() +
                       " to " + type.toString());
        }
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

    If(std::shared_ptr<Node> condition, std::shared_ptr<Node> bodyPositive,
       std::shared_ptr<Node> bodyNegative, const SourcePosition& thePosition)
        : Node(thePosition),
          condition(condition),
          bodyPositive(bodyPositive),
          bodyNegative(bodyNegative) {}

    std::shared_ptr<Node> getCondition() { return condition; }
    std::shared_ptr<Node> getPositive() { return bodyPositive; }
    std::shared_ptr<Node> getNegative() { return bodyNegative; }

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

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }
};

class While : public Node {
   private:
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> body;

   public:
    While(std::shared_ptr<Node> condition, std::shared_ptr<Node> body)
        : Node(), condition(condition), body(body) {}

    While(std::shared_ptr<Node> condition, std::shared_ptr<Node> body,
          const SourcePosition& thePosition)
        : Node(thePosition), condition(condition), body(body) {}

    std::shared_ptr<Node> getCondition() { return condition; }
    std::shared_ptr<Node> getBody() { return body; }

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

    virtual DataType getDataType() override {
        return DataType::Primitive::None;
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

    Literal(const std::string& value, DataType dataType,
            const SourcePosition& thePosition)
        : Node(thePosition), value(value) {
        this->dataType = dataType;
    }

    Literal(const std::string& value, std::string& dataTypeStr,
            const SourcePosition& thePosition)
        : Node(thePosition), value(value) {
        this->dataType = dataTypeStr;
    }

    virtual void toString(std::stringstream& stream) override {
        stream << dataType.toString() << "('" << value << "')";
    }

    virtual NodeType getType() override { return NodeType::Literal; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }

    int getIntValue() { return std::stoi(value); }
    int getFloatValue() { return std::stof(value); }
    bool getBoolValue() { return value == "true"; }
    std::string getStringValue() { return value; }
};

class Function : public Node {
   private:
    std::shared_ptr<Declfn> head;
    std::shared_ptr<Node> body;

   public:
    Function(std::shared_ptr<Declfn> head, std::shared_ptr<Node> body)
        : head(head), body(body) {}

    Function(std::shared_ptr<Declfn> head, std::shared_ptr<Node> body,
             const SourcePosition& thePosition)
        : Node(thePosition), head(head), body(body) {}

    std::shared_ptr<Declfn> getHead() { return head; }
    std::shared_ptr<Node> getBody() { return body; }

    void setBody(std::shared_ptr<Node> newBody) { body = newBody; }

    virtual void toString(std::stringstream& stream) override {
        stream << "function(";
        head->toString(stream);
        stream << ", ";
        body->toString(stream);
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::Function; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {head, body};
    }
    virtual DataType getDataType() override {
        return DataType::Primitive::None;
    }
};

class FnPtr : public Node {
   private:
    std::string id;

   public:
    FnPtr(const std::string& id, DataType dataType) : id(id) {
        this->dataType = dataType;
    }

    FnPtr(const std::string& id, DataType dataType,
          const SourcePosition& thePosition)
        : Node(thePosition), id(id) {
        this->dataType = dataType;
    }

    std::string getId() const { return id; }

    virtual void toString(std::stringstream& stream) override {
        stream << "fnptr(" << id << ", " << dataType.toString() << ")";
    }

    virtual NodeType getType() override { return NodeType::FnPtr; }
    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        return {};
    }
};

class DeclStruct : public Node {
   private:
    std::shared_ptr<Identifier> name;
    std::vector<std::shared_ptr<Declvar>> members;

   public:
    DeclStruct(std::shared_ptr<Identifier> name, const SourcePosition& thePosition)
        : Node(thePosition), name(name) {}

    std::shared_ptr<Identifier> getIdentifier() { return name; }
    std::vector<std::shared_ptr<Declvar>>& getMembers() { return members; }

    void addMember(std::shared_ptr<Declvar> member) {
        members.push_back(member);
    }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "declstruct(";
        name->toString(stream);
        stream << ", members(";
        for (const auto& member : members) {
            member->toString(stream);
            stream << " ";
        }
        stream << "))";
    }

    virtual NodeType getType() override { return NodeType::DeclStruct; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(members.size() + 1);
        vec.emplace_back(name);
        for (const auto& member : members) {
            vec.emplace_back(member);
        }
        return vec;
    }

    virtual DataType getDataType() override {
        return DataType::Primitive::None; // TODO
    }
};


class StructAccess : public Node {
   private:
    std::vector<std::shared_ptr<Identifier>> identifiers;

   public:
    StructAccess(const std::vector<std::shared_ptr<Identifier>>& identifiers, const SourcePosition& thePosition)
        : Node(thePosition), identifiers(identifiers) {}

    std::vector<std::shared_ptr<Identifier>>& getIdentifiers() { return identifiers; }

    virtual void toString(std::stringstream& stream) override {
        stream << getDataTypeString() << "struct_access(";
        for (const auto& identifier : identifiers) {
            identifier->toString(stream);
            stream << " ";
        }
        if (!identifiers.empty())
            stream.seekp(-1, std::ios_base::end); // Remove last space
        stream << ")";
    }

    virtual NodeType getType() override { return NodeType::StructAccess; }

    virtual std::vector<std::shared_ptr<Node>> getChildren() override {
        std::vector<std::shared_ptr<Node>> vec;
        vec.reserve(identifiers.size());
        for (const auto& id : identifiers) {
            vec.emplace_back(id);
        }
        return vec;
    }

    void setDataType(DataType type, AddMsgFn addMessage) {
        if (dataType == DataType::Primitive::Unknown || dataType == type)
            dataType = type;
        else {
            dataType = DataType::Primitive::Conflict;
            addMessage("Conflicting types: set " + dataType.toString() +
                       " to " + type.toString());
        }
    }

    // TODO: Type of last identifier in the list
    /*virtual DataType getDataType() override {
        return DataType::Primitive::None; // TODO
    }*/
};

}  // namespace AST
