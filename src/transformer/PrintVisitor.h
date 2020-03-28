#import <iostream>
#import <sstream>
#import <string>

using namespace MGrammar;

class PrintVisitor : public MGrammarBaseVisitor {
   private:
    unsigned int indentation;
    std::stringstream out;

   public:
    PrintVisitor() : indentation(0) {}

    std::string toString() { return out.str(); }

    virtual antlrcpp::Any visitStatement(
        MGrammarParser::StatementContext *ctx) override {
        for (unsigned int i = 0u; i < indentation; ++i) {
            out << "  ";
        }
        auto result = visitChildren(ctx);
        out << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitRet(MGrammarParser::RetContext *ctx) override {
        out << "ret(";
        auto result = visitChildren(ctx);
        out << ")";
        return result;
    }

    virtual antlrcpp::Any visitAssignment(
        MGrammarParser::AssignmentContext *ctx) override {
        out << "assign(";
        /*auto left = */ visit(ctx->assignment_left());
        out << ", ";
        auto right = visit(ctx->assignment_right());
        out << ")";
        return right;
    }

    virtual antlrcpp::Any visitVariable_decl(
        MGrammarParser::Variable_declContext *ctx) override {
        out << "let(";
        auto result = visitChildren(ctx);
        out << ")";
        return result;
    }

    virtual antlrcpp::Any visitIdentifier_list(
        MGrammarParser::Identifier_listContext *ctx) override {
        const auto &idents = ctx->identifier();
        for (auto it = idents.begin(); it != idents.end(); ++it) {
            visit(*it);
            if (std::next(it) != idents.end()) {
                out << ", ";
            }
        }
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitFunction_decl(
        MGrammarParser::Function_declContext *ctx) override {
        out << "let('";
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

    virtual antlrcpp::Any visitInfix_call(
        MGrammarParser::Infix_callContext *ctx) override {
        out << "call('";
        visit(ctx->identifier());
        out << "', ";
        visit(ctx->infix_call_left());
        out << ", ";
        visit(ctx->infix_call_right());
        out << ")";
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitIdentifier(
        MGrammarParser::IdentifierContext *ctx) override {
        out << ctx->getText();
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitArgument_list(
        MGrammarParser::Argument_listContext *ctx) override {
        const auto &args = ctx->expr();
        for (auto it = args.begin(); it != args.end(); ++it) {
            visit(*it);
            if (std::next(it) != args.end()) {
                out << ", ";
            }
        }
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitCall(MGrammarParser::CallContext *ctx) override {
        out << "call('" << ctx->identifier()->getText() << "'";
        if (ctx->argument_list()) {
            out << ", ";
            visit(ctx->argument_list());
        }
        out << ")";
        return antlrcpp::Any(ctx);
    }

    virtual antlrcpp::Any visitBlock(
        MGrammarParser::BlockContext *ctx) override {
        out << "{" << std::endl;
        ++indentation;

        auto result = visitChildren(ctx);

        --indentation;
        out << "}";
        return result;
    }

    virtual antlrcpp::Any visitLiteral(
        MGrammarParser::LiteralContext *ctx) override {
        out << ctx->getText();
        return antlrcpp::Any(ctx);
    }
};
