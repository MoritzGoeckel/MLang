#include "PrintVisitor.h"

#include <iostream>
#include <sstream>

using namespace MGrammar;

PrintVisitor::PrintVisitor() : indentation(0) {}

std::string PrintVisitor::toString() { return out.str(); }

antlrcpp::Any PrintVisitor::visitRet(MGrammarParser::RetContext *ctx) {
    out << "ret(";
    auto result = visitChildren(ctx);
    out << ")";
    return result;
}

antlrcpp::Any PrintVisitor::visitAssignment(
    MGrammarParser::AssignmentContext *ctx) {
    out << "assign(";
    /*auto left = */ visit(ctx->assignment_left());
    out << ", ";
    auto right = visit(ctx->assignment_right());
    out << ")";
    return right;
}

antlrcpp::Any PrintVisitor::visitVariable_decl(
    MGrammarParser::Variable_declContext *ctx) {
    out << "letvar(";
    auto result = visitChildren(ctx);
    out << ")";
    return result;
}

antlrcpp::Any PrintVisitor::visitIdentifier_list(
    MGrammarParser::Identifier_listContext *ctx) {
    const auto &idents = ctx->identifier();
    for (auto it = idents.begin(); it != idents.end(); ++it) {
        visit(*it);
        if (std::next(it) != idents.end()) {
            out << ", ";
        }
    }
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitFunction_decl(
    MGrammarParser::Function_declContext *ctx) {
    out << "letfn('";
    visit(ctx->identifier());
    out << "'";
    if (ctx->identifier_list()) {
        out << ", params(";
        visit(ctx->identifier_list());
        out << ")";
    }
    out << ")";
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitInfix_call(
    MGrammarParser::Infix_callContext *ctx) {
    out << "call('";
    visit(ctx->identifier());
    out << "', ";
    visit(ctx->infix_call_left());
    out << ", ";
    visit(ctx->infix_call_right());
    out << ")";
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitIdentifier(
    MGrammarParser::IdentifierContext *ctx) {
    out << ctx->getText();
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitArgument_list(
    MGrammarParser::Argument_listContext *ctx) {
    const auto &args = ctx->expr();
    for (auto it = args.begin(); it != args.end(); ++it) {
        visit(*it);
        if (std::next(it) != args.end()) {
            out << ", ";
        }
    }
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitCall(MGrammarParser::CallContext *ctx) {
    out << "call('" << ctx->identifier()->getText() << "'";
    if (ctx->argument_list()) {
        out << ", ";
        visit(ctx->argument_list());
    }
    out << ")";
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitBlock(MGrammarParser::BlockContext *ctx) {
    const auto &statements = ctx->statement();
    if (statements.size() == 1u) {
        return visit(statements[0]);
    }

    if (statements.empty()) {
        return antlrcpp::Any(ctx);
    }

    auto parent = static_cast<antlr4::ParserRuleContext *>(ctx->parent);

    // Dont indent if it is the most outer block
    if (parent->getRuleIndex() != MGrammarParser::RuleR) {
        out << "{" << std::endl;
        ++indentation;
    }

    // Generate indentation
    std::string indent;
    for (unsigned int i = 0u; i < indentation; ++i) {
        indent += "  ";
    }

    for (auto &s : statements) {
        out << indent;
        visit(s);
        out << std::endl;
    }

    // Dont indent if it is the most outer block
    if (parent->getRuleIndex() != MGrammarParser::RuleR) {
        --indentation;
        out << "}";
    }
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitLiteral(MGrammarParser::LiteralContext *ctx) {
    std::string type;
    if (ctx->type_float())
        type = "float";
    else if (ctx->type_int())
        type = "int";
    else if (ctx->type_bool())
        type = "bool";
    else if (ctx->type_string())
        type = "string";
    else
        throw "Unknown type";

    out << type << "('" << ctx->getText() << "')";
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitBranching_if(
    MGrammarParser::Branching_ifContext *ctx) {
    out << "if(";
    visit(ctx->expr());
    out << ", ";
    visit(ctx->positive());
    if (ctx->negative()) {
        out << ", ";
        visit(ctx->negative());
    }
    out << ")";
    return antlrcpp::Any(ctx);
}

antlrcpp::Any PrintVisitor::visitBranching_while(
    MGrammarParser::Branching_whileContext *ctx) {
    out << "while(";
    visit(ctx->expr());
    out << ", ";
    visit(ctx->statement());
    out << ")";

    return antlrcpp::Any(ctx);
}
