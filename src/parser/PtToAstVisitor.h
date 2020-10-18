#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "MGrammarBaseListener.h"
#include "MGrammarBaseVisitor.h"
#include "MGrammarLexer.h"
#include "MGrammarParser.h"
#include "antlr4-runtime.h"

#include "../ast/DataType.h"
#include "../ast/Node.h"

using namespace MGrammar;

class PtToAstVisitor : public MGrammarBaseVisitor {
   private:
    // TODO: Use return instead of stack
    std::vector<std::shared_ptr<AST::Node>> stack;

   public:
    PtToAstVisitor() {}

    std::shared_ptr<AST::Node> getAST();

    virtual antlrcpp::Any visitRet(MGrammarParser::RetContext *ctx) override;

    virtual antlrcpp::Any visitAssignment(
        MGrammarParser::AssignmentContext *ctx) override;

    virtual antlrcpp::Any visitVariable_decl(
        MGrammarParser::Variable_declContext *ctx) override;

    virtual antlrcpp::Any visitIdentifier_list(
        MGrammarParser::Identifier_listContext *ctx) override;

    virtual antlrcpp::Any visitFunction_decl(
        MGrammarParser::Function_declContext *ctx) override;

    virtual antlrcpp::Any visitInfix_call(
        MGrammarParser::Infix_callContext *ctx) override;

    virtual antlrcpp::Any visitIdentifier(
        MGrammarParser::IdentifierContext *ctx) override;

    virtual antlrcpp::Any visitArgument_list(
        MGrammarParser::Argument_listContext *ctx) override;

    virtual antlrcpp::Any visitCall(MGrammarParser::CallContext *ctx) override;

    virtual antlrcpp::Any visitBlock(
        MGrammarParser::BlockContext *ctx) override;

    virtual antlrcpp::Any visitLiteral(
        MGrammarParser::LiteralContext *ctx) override;

    virtual antlrcpp::Any visitBranching_if(
        MGrammarParser::Branching_ifContext *ctx) override;

    virtual antlrcpp::Any visitBranching_while(
        MGrammarParser::Branching_whileContext *ctx) override;
};
