#include <iostream>

#include "MGrammarBaseListener.h"
#include "MGrammarBaseVisitor.h"
#include "MGrammarLexer.h"
#include "MGrammarParser.h"

using namespace std;
using namespace antlr4;
using namespace MGrammar;
using namespace antlrcpp;

class MListener : public MGrammarBaseListener {
    virtual void enterR(MGrammarParser::RContext* ctx) override {
        std::cout << "jo!" << std::endl;
    }
};

class MVisitor : public MGrammarBaseVisitor {
    virtual Any visitR(MGrammarParser::RContext* context) {
        std::cout << "joo!" << std::endl;
        return antlrcpp::Any(context);  // Dont forget to return
    }
};

int main() {
    std::string src("int test = 10;");
    ANTLRInputStream input(src.c_str(), src.size());
    MGrammarLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }

    MGrammarParser parser(&tokens);
    tree::ParseTree* tree = parser.r();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    // Run listener
    MListener listener;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    // Run visitor
    MVisitor visitor;
    visitor.visit(tree);

    return 0;
}
