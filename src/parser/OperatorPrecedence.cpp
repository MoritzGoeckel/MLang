#include "OperatorPrecedence.h"
#include <string>

int precedence(std::shared_ptr<AST::Identifier> theOperator) {
    auto name = theOperator->getName();
    int defaultPrecedence = 0;

    if (name.size() == 1u) {
        // It's a char
        char c = name[0];
        switch (c) {
            case '+':
                return 0;
            case '-':
                return 0;
            case '*':
                return 1;
            case '/':
                return 1;
            case '%':
                return 2;
            case '^':
                return 2;
            default:
                return defaultPrecedence;
        }
    }

    // Handle operators with size > 1 here

    return defaultPrecedence;
}
