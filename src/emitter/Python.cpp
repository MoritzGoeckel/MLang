#include "Python.h"

#include "../Logger.h"


namespace emitter{

Python::Python(const std::map<std::string, std::shared_ptr<AST::Function>> &functions)
    : Emitter(functions), code{}, indent_level{0} {
        build_in_functions = {
            {"print", "_print"},
            {"+", "_add"},
            {"-", "_sub"},
            {"*", "_mul"},
            {"/", "_div"},
            {"<", "_lt"},
            {">", "_gt"},
            {"==", "_eq"},
        };
    }

void Python::run() {
    for (auto &fn : functions) {
        instantiateFn(fn.first, fn.second);
    }

    implement_build_in_functions();
}

std::string Python::toString() {
    return code.str();
}

std::string Python::formatFnName(const std::string &name) {
    std::string formattedName;
    char lastChar = '\0';
    for (char c : name) {
        // alphanumeric characters and underscores are allowed
        if (std::isalnum(c) || c == '_') {
            lastChar = c;
            formattedName += c;
        } else if (lastChar != '_' && lastChar != '\0') {
            formattedName += '_';
            lastChar = '_';
        }
    }
    return formattedName;
}


void Python::instantiateFn(const std::string &name, std::shared_ptr<AST::Function> ast) {
    indent();
    code << "def " << formatFnName(name) << "(";
    for(const auto& param : ast->getHead()->getParameters()){
        code << param->getName() << ", ";
    }

    // Remove the last comma if there are parameters
    if (!ast->getHead()->getParameters().empty()) {
        code.seekp(-2, std::ios_base::end);
    }
    code << "):\n";
    indent_level++;
    process(ast->getBody()); 
    indent_level--;
    code << "\n";
}

void Python::process_inline(std::shared_ptr<AST::Node> node) {
    // Don't indent here, it should be just a line
    switch(node->getType()) {
        case AST::NodeType::Call: {
            auto call = std::dynamic_pointer_cast<AST::Call>(node);
            std::string fnName = call->getIdentifier()->getName();

            // Rename built-in functions and remember used ones
            auto it = build_in_functions.find(fnName);
            if(it != build_in_functions.end()) {
                used_build_in_functions.insert(fnName);
                fnName = it->second;
            }
            
            code << fnName << "(";
            for(const auto& arg : call->getArguments()) {
                process_inline(arg);
                code << ", ";
            }
            // Remove the last comma if there are arguments
            if (!call->getArguments().empty()) {
                code.seekp(-2, std::ios_base::end);
            }
            code << ")";
            break;
        }
        case AST::NodeType::Literal: {
            auto literal = std::dynamic_pointer_cast<AST::Literal>(node);
            if(literal->getDataType() == DataType::Primitive::String){
                code << "'" << literal->getStringValue() << "'";
            } else if (literal->getDataType() == DataType::Primitive::Bool) {
                code << (literal->getBoolValue() ? "True" : "False");
            }
            else {
                code << literal->getStringValue();
            }
            break;
        }
        case AST::NodeType::Identifier: {
            auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
            code << identifier->getName();
            break;
        }
        case AST::NodeType::Declvar: {
            auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
            code << declvar->getIdentifier()->getName();
            break;
        }
        case AST::NodeType::FnPtr: {
            auto fnPtr = std::dynamic_pointer_cast<AST::FnPtr>(node);
            code << fnPtr->getId();
            break;
        }
        default: {
            // Handle other node types as needed
            throw "Unsupported node type for expression processing";
            // TODO use messages
        }
    }
}

void Python::process(const std::shared_ptr<AST::Node>& node) {
    switch(node->getType()) {
        case AST::NodeType::Block: {
            followChildren(node);
            break;
        }
        case AST::NodeType::Ret: {
            // TODO: Somehow we miss a return sometimes (simple_fns.m)
            indent();
            auto ret = std::dynamic_pointer_cast<AST::Ret>(node);
            if (ret->getExpr()) {
                code << "return ";
                process_inline(ret->getExpr());
            } else {
                code << "return None";
            }
            code << "\n";
            break;
        }
        case AST::NodeType::Assign: {
            auto assign = std::dynamic_pointer_cast<AST::Assign>(node);
            indent();
            process_inline(assign->getLeft());
            code << " = ";
            process_inline(assign->getRight());
            code << "\n";
            break;
        }
        case AST::NodeType::If: {
            auto ifNode = std::dynamic_pointer_cast<AST::If>(node);
            indent();
            code << "if ";
            process_inline(ifNode->getCondition());
            code << ":\n";
            indent_level++;
            followChildren(ifNode->getPositive());
            indent_level--;
            if (ifNode->getNegative()) {
                indent();
                code << "else:\n";
                indent_level++;
                followChildren(ifNode->getNegative());
                indent_level--;
            }
            break;
        }
        default: {
            indent();
            process_inline(node);
            code << "\n";
            break;
        }
    }
}

void Python::indent() {
    for (int i = 0; i < indent_level; ++i) {
        code << "    ";
    }
}

void Python::implement_build_in_functions() {
    std::stringstream stream;
    stream << "# Built-in functions\n";
    for (const auto& fn : used_build_in_functions) {
        if(fn == "+"){
            stream << "def _add(a, b):\n    return a + b\n";
        } 
        else if(fn == "-"){
            stream << "def _sub(a, b):\n    return a - b\n";
        }
        else if(fn == "*"){
            stream << "def _mul(a, b):\n    return a * b\n";
        }
        else if(fn == "/"){
            stream << "def _div(a, b):\n    return a / b\n";
        }
        else if(fn == "<"){
            stream << "def _lt(a, b):\n    return a < b\n";
        }
        else if(fn == ">"){
            stream << "def _gt(a, b):\n    return a > b\n";
        }
        else if(fn == "=="){
            stream << "def _eq(a, b):\n    return a == b\n";
        }
        else if(fn == "print"){
            stream << "def _print(args):\n    print(args)\n";
        }
    }
    stream << "# End of built-in functions\n\n";
    stream << code.str();
    code = std::move(stream);
}
} // namespace emitter