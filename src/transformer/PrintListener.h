#import <iostream>

using namespace MGrammar;

class PrintListener : public MGrammarBaseListener {
   private:
    const std::vector<std::string>& ruleNames;
    unsigned int indentation = 0u;

   public:
    PrintListener(const std::vector<std::string>& ruleNames)
        : ruleNames(ruleNames) {}

    virtual void enterR(MGrammarParser::RContext* ctx) override {
        indentation = 0u;
        // std::cout << std::endl;
    }
    virtual void enterEveryRule(antlr4::ParserRuleContext* ctx) override {
        // std::cout << ruleNames[ctx->getRuleIndex()] << "(";
        // std::cout << ctx->toString() << " | ";
        // std::cout << ctx.getStart()->getText();
    }

    virtual void enterRet(MGrammarParser::RetContext* ctx) override {
        std::cout << "ret(";
    }
    virtual void exitRet(MGrammarParser::RetContext* ctx) override {
        std::cout << ")";
    }

    virtual void exitStatement(MGrammarParser::StatementContext* ctx) override {
        std::cout << std::endl;
    }

    virtual void enterAssignment(
        MGrammarParser::AssignmentContext* ctx) override {
        std::cout << "assignment(";
    }
    virtual void exitAssignment(
        MGrammarParser::AssignmentContext* ctx) override {
        std::cout << ")";
    }

    virtual void exitAssignment_left(
        MGrammarParser::Assignment_leftContext* ctx) override {
        std::cout << ", ";
    }
    virtual void enterVariable_decl(
        MGrammarParser::Variable_declContext* ctx) override {
        std::cout << "let ";
    }

    virtual void enterIdentifier(
        MGrammarParser::IdentifierContext* ctx) override {
        if (ctx->parent != nullptr &&
            static_cast<antlr4::RuleContext*>(ctx->parent)->getRuleIndex() !=
                MGrammarParser::RuleInfix_call &&
            static_cast<antlr4::RuleContext*>(ctx->parent)->getRuleIndex() !=
                MGrammarParser::RuleCall) {
            std::cout << ctx->getText() << " ";
        }
    }

    virtual void enterInfix_call(
        MGrammarParser::Infix_callContext* ctx) override {
        std::cout << "call('" << ctx->identifier()->getText() << "', ";
    }
    virtual void exitInfix_call(
        MGrammarParser::Infix_callContext* ctx) override {
        std::cout << ")";
    }

    virtual void enterCall(MGrammarParser::CallContext* ctx) override {
        std::cout << "call('" << ctx->identifier()->getText() << "', ";
    }
    virtual void exitCall(MGrammarParser::CallContext* ctx) override {
        std::cout << ")";
    }

    virtual void enterBlock(MGrammarParser::BlockContext* /*ctx*/) override {
        std::cout << "{";
        ++indentation;
    }
    virtual void exitBlock(MGrammarParser::BlockContext* /*ctx*/) override {
        --indentation;
        std::cout << "}";
    }

    virtual void enterLiteral(MGrammarParser::LiteralContext* ctx) override {
        std::cout << ctx->getText() << " ";
    }

    virtual void exitR(MGrammar::MGrammarParser::RContext* ctx) override {
        std::cout << std::endl << std::endl;
    }
};

