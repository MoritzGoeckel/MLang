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
        out << "letvar(";
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

    virtual antlrcpp::Any visitLiteral(
        MGrammarParser::LiteralContext *ctx) override {
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

    virtual antlrcpp::Any visitBranching_if(
        MGrammarParser::Branching_ifContext *ctx) override {
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

    virtual antlrcpp::Any visitBranching_while(
        MGrammarParser::Branching_whileContext *ctx) override {
        out << "while(";
        visit(ctx->expr());
        out << ", ";
        visit(ctx->statement());
        out << ")";

        return antlrcpp::Any(ctx);
    }
};
