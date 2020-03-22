class MListener : public MGrammar::MGrammarBaseListener {
    virtual void enterR(MGrammar::MGrammarParser::RContext* ctx) override {
        std::cout << "jo!" << std::endl;
    }
};

class MVisitor : public MGrammar::MGrammarBaseVisitor {
    virtual antlrcpp::Any visitR(MGrammar::MGrammarParser::RContext* context) {
        std::cout << "joo!" << std::endl;
        return antlrcpp::Any(context);  // Dont forget to return
    }
};

