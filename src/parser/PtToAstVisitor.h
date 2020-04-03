#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../ast/Node.h"

using namespace MGrammar;

class PtToAstVisitor : public MGrammarBaseVisitor {
   private:
    // TODO: Use return instead of stack
    std::vector<std::shared_ptr<AST::Node>> stack;

   public:
    PtToAstVisitor() {}

    std::shared_ptr<AST::Node> getAST() {
        if (stack.empty()) throw "Empty stack";
        if (stack.size() != 1u)
            throw "Bad stack size: " + std::to_string(stack.size());

        return stack.front();
    }

    virtual antlrcpp::Any visitRet(MGrammarParser::RetContext *ctx) override {
        if (ctx->expr()) {
            visit(ctx->expr());
            auto expr = stack.back();
            stack.pop_back();
            stack.push_back(std::make_shared<AST::Ret>(expr));
        } else {
            stack.push_back(std::make_shared<AST::Ret>());
        }
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitAssignment(
        MGrammarParser::AssignmentContext *ctx) override {
        visit(ctx->assignment_left());
        auto left = stack.back();
        stack.pop_back();

        visit(ctx->assignment_right());
        auto right = stack.back();
        stack.pop_back();

        stack.push_back(std::make_shared<AST::Assign>(left, right));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitVariable_decl(
        MGrammarParser::Variable_declContext *ctx) override {
        visit(ctx->identifier());
        auto identifier = stack.back();
        stack.pop_back();

        stack.push_back(std::make_shared<AST::Declvar>(identifier));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitIdentifier_list(
        MGrammarParser::Identifier_listContext *ctx) override {
        const auto &idents = ctx->identifier();
        for (auto it = idents.begin(); it != idents.end(); ++it) {
            visit(*it);
        }
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitFunction_decl(
        MGrammarParser::Function_declContext *ctx) override {
        visit(ctx->identifier());
        auto identifier = stack.back();
        stack.pop_back();

        std::vector<std::shared_ptr<AST::Node>> parameters;
        if (ctx->identifier_list()) {
            unsigned int beforeStackSize = stack.size();

            visit(ctx->identifier_list());

            while (stack.size() > beforeStackSize) {
                parameters.push_back(stack.back());
                stack.pop_back();
            }
        }

        stack.push_back(std::make_shared<AST::Declfn>(identifier, parameters));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitInfix_call(
        MGrammarParser::Infix_callContext *ctx) override {
        visit(ctx->identifier());
        auto identifier = stack.back();
        stack.pop_back();

        visit(ctx->infix_call_left());
        auto left = stack.back();
        stack.pop_back();

        visit(ctx->infix_call_right());
        auto right = stack.back();
        stack.pop_back();

        stack.push_back(std::make_shared<AST::Call>(
            identifier,
            std::vector<std::shared_ptr<AST::Node>>({left, right})));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitIdentifier(
        MGrammarParser::IdentifierContext *ctx) override {
        stack.push_back(std::make_shared<AST::Identifier>(ctx->getText()));
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitArgument_list(
        MGrammarParser::Argument_listContext *ctx) override {
        const auto &args = ctx->expr();
        for (auto it = args.begin(); it != args.end(); ++it) {
            visit(*it);
        }
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitCall(MGrammarParser::CallContext *ctx) override {
        visit(ctx->identifier());
        auto identifier = stack.back();
        stack.pop_back();

        std::vector<std::shared_ptr<AST::Node>> arguments;
        if (ctx->argument_list()) {
            unsigned int beforeStackSize = stack.size();
            visit(ctx->argument_list());

            while (stack.size() > beforeStackSize) {
                arguments.push_back(stack.back());
                stack.pop_back();
            }
        }
        stack.push_back(std::make_shared<AST::Call>(identifier, arguments));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitBlock(
        MGrammarParser::BlockContext *ctx) override {
        const auto &statements = ctx->statement();
        if (statements.size() == 1u) {
            return visit(statements[0]);
        }

        if (statements.empty()) {
            return antlrcpp::Any(ctx);
        }

        std::vector<std::shared_ptr<AST::Node>> children;
        for (auto &s : statements) {
            visit(s);
            children.push_back(stack.back());
            stack.pop_back();
        }

        stack.push_back(std::make_shared<AST::Block>(children));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitLiteral(
        MGrammarParser::LiteralContext *ctx) override {
        AST::DataType type;
        if (ctx->type_float())
            type = AST::DataType::Float;
        else if (ctx->type_int())
            type = AST::DataType::Int;
        else if (ctx->type_bool())
            type = AST::DataType::Bool;
        else if (ctx->type_string())
            type = AST::DataType::String;
        else
            throw "Unknown type";

        stack.push_back(std::make_shared<AST::Literal>(ctx->getText(), type));
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitBranching_if(
        MGrammarParser::Branching_ifContext *ctx) override {
        visit(ctx->expr());
        auto condition = stack.back();
        stack.pop_back();

        visit(ctx->positive());
        auto bodyPositive = stack.back();
        stack.pop_back();

        std::shared_ptr<AST::Node> bodyNegative;
        if (ctx->negative()) {
            visit(ctx->negative());
            bodyNegative = stack.back();
            stack.pop_back();
        }

        stack.push_back(
            std::make_shared<AST::If>(condition, bodyPositive, bodyNegative));

        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitBranching_while(
        MGrammarParser::Branching_whileContext *ctx) override {
        visit(ctx->expr());
        auto condition = stack.back();
        stack.pop_back();

        visit(ctx->statement());
        auto body = stack.back();
        stack.pop_back();

        stack.push_back(std::make_shared<AST::While>(condition, body));

        return antlrcpp::Any(ctx);
    }
};
