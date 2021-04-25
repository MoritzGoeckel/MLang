#include "Parser.h"

#include <deque>
#include "OperatorPrecedence.h"

Parser::Parser(std::vector<Token>&& tokens)
    : cache(), itsAst(nullptr), tokens(std::move(tokens)), markers(), idx(0u) {
    itsAst = file();
}

Parser::Parser(const std::vector<Token>& tokens)
    : cache(), itsAst(nullptr), tokens(tokens), markers(), idx(0u) {
    itsAst = file();
}

std::shared_ptr<AST::Node> Parser::getAst() { return itsAst; }

std::string Parser::getError() {
    if (itsAst) return "";
    return itsError.second;
}

Parser::CacheResult Parser::checkCache(size_t index, Parser::Rule rule) {
    auto aFoundEntry = cache.find(index);
    if (aFoundEntry == cache.end()) {
        return MISS;
    }

    auto aFoundIndex = aFoundEntry->second.find(rule);
    if (aFoundIndex == aFoundEntry->second.end()) {
        return MISS;
    }

    return aFoundIndex->second;
}

void Parser::memorize(size_t index, Parser::Rule rule,
                      Parser::CacheResult result) {
    cache[index][rule] = result;  // Result could be stop index
}

void Parser::mark() { markers.push_back(idx); }

void Parser::reset() {
    idx = markers.back();
    markers.pop_back();
}

Token Parser::lookAhead(size_t offset) {
    if (idx + offset >= tokens.size()) return Token();
    return tokens[idx + offset];
}
Token::Type Parser::lookAhead_t(size_t offset) {
    if (idx + offset >= tokens.size()) return Token::Type::None;
    return tokens[idx + offset].getType();
}

Token Parser::nextToken() { return lookAhead(0u); }

bool Parser::isNext(Token::Type type) { return lookAhead_t(0) == type; }

bool Parser::isNext(char c) {
    if (idx >= tokens.size()) return false;
    return tokens[idx].getChar() == c;
}

Token Parser::consume() { return tokens[idx++]; }

bool Parser::consume(char c) {
    if (idx < tokens.size() && tokens[idx].getChar() == c) {
        ++idx;
        return true;
    } else {
        return false;
    }
}

bool Parser::consume(Token::Type expectedType) {
    if (idx < tokens.size() && tokens[idx].getType() == expectedType) {
        ++idx;
        return true;
    } else {
        return false;
    }
}

#define doOrFailMessage(action, expected, failMsg) \
    do {                                           \
        if (!action) {                             \
            report(expected, failMsg);             \
            return nullptr;                        \
        }                                          \
    } while (0)

#define doOrFail(action, expected) \
    do {                           \
        if (!action) {             \
            report(expected, "");  \
            return nullptr;        \
        }                          \
    } while (0)

#define consumeOrFail(tokenType, expected) \
    doOrFail(consume(tokenType), expected)

#define fail(failMsg)    \
    do {                 \
        report(failMsg); \
        return nullptr;  \
    } while (0)

void Parser::report(std::string msg) {
    if (idx < itsError.first) return;
    itsError = std::make_pair(idx, "  " + msg);
}

void Parser::report(std::string expected, std::string msg) {
    if (idx < itsError.first) return;

    std::string errorMsg = "  Expected '" + expected + "' but found '" +
                           nextToken().getContent() + "'";

    if (!msg.empty()) errorMsg += "\n  " + msg;

    // TODO remove
    // errorMsg += " idx=" + std::to_string(idx);

    itsError = std::make_pair(idx, errorMsg);
}

bool Parser::isDone() { return idx == tokens.size(); }

std::vector<std::shared_ptr<AST::Node>> Parser::statementList() {
    std::vector<std::shared_ptr<AST::Node>> statements;
    while (speculate(&Parser::statement, Parser::Rule::Statement)) {
        statements.push_back(statement());
    }
    return statements;
}

std::shared_ptr<AST::Node> Parser::file() {
    auto statements = statementList();
    doOrFailMessage(isDone(), "EOF", "Could not consume all tokens");
    return std::make_shared<AST::Block>(statements);
}

std::shared_ptr<AST::Node> Parser::statement() {
    if (speculate(&Parser::ret, Parser::Rule::Ret)) {
        return ret();
    }

    if (speculate(&Parser::block, Parser::Rule::Block)) {
        return block();
    }

    if (speculate(&Parser::branching, Parser::Rule::Branching)) {
        return branching();
    }

    if (speculate(&Parser::expression, Parser::Rule::Expression)) {
        auto node = expression();
        doOrFailMessage(
            consume(Token::Type::StatementTerminator), ";",
            "Consider adding a semicolon to the end of the statement");
        return node;
    }

    fail("Expecting ret, block, branching or an expression");
}

std::shared_ptr<AST::Node> Parser::expression() {
    if (speculate(&Parser::infixCall, Parser::Rule::InfixCall)) {
        return infixCall();
    }

    if (speculate(&Parser::nrExpression, Parser::Rule::NrExpression)) {
        return nrExpression();
    }

    fail("Expecting infix call or non-recursive expression");
}

std::shared_ptr<AST::Node> Parser::parenthesizedExpression() {
    consumeOrFail('(', "(");
    doOrFail(speculate(&Parser::expression, Parser::Rule::Expression),
             "expression");
    auto expr = expression();
    consumeOrFail(')', ")");
    return expr;
}

std::shared_ptr<AST::Node> Parser::nrExpression() {
    if (speculate(&Parser::parenthesizedExpression,
                  Parser::Rule::ParenthesizedExpression)) {
        return parenthesizedExpression();
    }

    // assignment
    if (speculate(&Parser::assignment, Parser::Rule::Assignment)) {
        return assignment();
    }

    // block
    if (speculate(&Parser::block, Parser::Rule::Block)) {
        return block();
    }

    // call
    if (speculate(&Parser::call, Parser::Rule::Call)) {
        return call();
    }

    // literal
    if (speculate(&Parser::literal, Parser::Rule::Literal)) {
        return literal();
    }

    // identifier
    if (speculate(&Parser::identifier, Parser::Rule::Identifier)) {
        return identifier();
    }

    fail("Failed to parse a non recursive expression");
}

std::shared_ptr<AST::Ret> Parser::ret() {
    consumeOrFail(Token::Type::Ret, "ret");

    if (consume(Token::Type::StatementTerminator)) {
        return std::make_shared<AST::Ret>();
    }

    doOrFail(speculate(&Parser::expression, Parser::Rule::Expression),
             "expression");
    auto expr = expression();
    consumeOrFail(Token::Type::StatementTerminator, ";");

    return std::make_shared<AST::Ret>(expr);
}

std::shared_ptr<AST::Block> Parser::block() {
    consumeOrFail('{', "{");
    auto statements = statementList();
    consumeOrFail('}', "}");
    return std::make_shared<AST::Block>(statements);
}

std::shared_ptr<AST::Call> Parser::call() {
    std::shared_ptr<AST::Identifier> method;
    std::vector<std::shared_ptr<AST::Node>> arguments;

    doOrFail(speculate(&Parser::identifier, Parser::Rule::Identifier),
             "identifier");
    method = identifier();

    consumeOrFail('(', "(");
    doOrFail(argumentList(arguments), "list of arguments");
    consumeOrFail(')', ")");

    return std::make_shared<AST::Call>(method, arguments);
}

bool Parser::argumentList(std::vector<std::shared_ptr<AST::Node>>& theList) {
    do {
        if (!speculate(&Parser::expression, Parser::Rule::Expression))
            return false;

        auto expr = expression();
        if (!expr) return false;
        theList.push_back(expr);
    } while (consume(Token::Type::Comma));
    return true;
}

// TODO: Identifier as args
bool Parser::identifierList(std::vector<std::shared_ptr<AST::Node>>& theList) {
    do {
        if (speculate(&Parser::identifier, Parser::Rule::Identifier)) {
            auto ident = identifier();
            if (!ident) return false;
            theList.push_back(ident);  // TODO move
        } else {
            return false;
        }
    } while (consume(Token::Type::Comma));
    return true;
}

std::shared_ptr<AST::Identifier> Parser::identifier() {
    doOrFail((isNext(Token::Type::Identifier) || isNext(Token::Type::Special)),
             "identifier or special");
    auto token = consume();
    return std::make_shared<AST::Identifier>(token.getContent());
}

std::shared_ptr<AST::Literal> Parser::literal() {
    if (speculate(&Parser::integer, Parser::Rule::Integer)) {
        return integer();
    }

    if (speculate(&Parser::boolean, Parser::Rule::Boolean)) {
        return boolean();
    }

    fail("Failed to parse literal");
}

std::shared_ptr<AST::Literal> Parser::integer() {
    doOrFail(isNext(Token::Type::Number), "number");
    auto token = consume();

    return std::make_shared<AST::Literal>(token.getContent(),
                                          DataType::Primitive::Int);
}

std::shared_ptr<AST::Call> Parser::infixCall() {
    // Create list of operands and operators
    bool shouldBeExpression = true;

    std::deque<std::shared_ptr<AST::Identifier>> operators;
    std::deque<std::shared_ptr<AST::Node>> operands;

    while ((speculate(&Parser::nrExpression, Parser::Rule::NrExpression) &&
            shouldBeExpression) ||
           (speculate(&Parser::identifier, Parser::Rule::Identifier) &&
            !shouldBeExpression)) {
        if (shouldBeExpression) {
            operands.push_back(nrExpression());
        } else {
            operators.push_back(identifier());
        }
        shouldBeExpression = !shouldBeExpression;
    }

    if (operands.size() < 2u || operators.empty() ||
        operands.size() != operators.size() + 1u) {
        fail(
            "Unexpected number of operands and operators in infix "
            "expression: " +
            std::to_string(operands.size()) + " operands and " +
            std::to_string(operators.size()) + " operators");
    }

    // Handle operator precedence
    while (!operators.empty()) {
        auto operandsIt = operands.begin();
        auto operatorsIt = operators.begin();
        auto lastOperatorIt = operators.end() - 1;

        while (operatorsIt != lastOperatorIt &&
               precedence(*operatorsIt) < precedence(*(operatorsIt + 1))) {
            ++operandsIt;
            ++operatorsIt;
        }

        auto reducedOperand = std::make_shared<AST::Call>(
            *operatorsIt, std::vector<std::shared_ptr<AST::Node>>{
                              *operandsIt, *(operandsIt + 1)});

        *operandsIt = reducedOperand;

        operands.erase(operandsIt + 1);
        operators.erase(operatorsIt);
    }

    if (operands.size() != 1u || !operators.empty()) {
        fail("Infix expression could not be reduced: " +
             std::to_string(operands.size()) + " operands and " +
             std::to_string(operators.size()) + " operators");
    }

    return std::dynamic_pointer_cast<AST::Call>(operands.front());
}

std::shared_ptr<AST::Assign> Parser::assignment() {
    doOrFail(speculate(&Parser::leftHandValue, Parser::Rule::LeftHandValue),
             "left hand value");
    auto left = leftHandValue();

    consumeOrFail(Token::Type::Assignment, "=");

    doOrFail(speculate(&Parser::expression, Parser::Rule::Expression),
             "expression");
    auto right = expression();

    return std::make_shared<AST::Assign>(left, right);
}

std::shared_ptr<AST::Node> Parser::leftHandValue() {
    if (speculate(&Parser::functionDecl, Parser::Rule::FunctionDecl)) {
        return functionDecl();
    }

    if (speculate(&Parser::variableDecl, Parser::Rule::VariableDecl)) {
        return variableDecl();
    }

    if (speculate(&Parser::identifier, Parser::Rule::Identifier)) {
        return identifier();
    }

    fail("Failed to parse left hand value");
}

std::shared_ptr<AST::Declvar> Parser::variableDecl() {
    consumeOrFail(Token::Type::Let, "let");
    doOrFail(speculate(&Parser::identifier, Parser::Rule::Identifier),
             "identifier");
    return std::make_shared<AST::Declvar>(identifier());
}

std::shared_ptr<AST::Declfn> Parser::functionDecl() {
    consumeOrFail(Token::Type::Let, "let");
    doOrFail(speculate(&Parser::identifier, Parser::Rule::Identifier),
             "identifier");
    auto method = identifier();
    consumeOrFail('(', "(");

    std::vector<std::shared_ptr<AST::Node>> params;
    if (!isNext(')')) {
        // Identifier list is optional
        doOrFail(identifierList(params), "list of identifiers");
    }

    consumeOrFail(')', ")");

    return std::make_shared<AST::Declfn>(method, params);
}

std::shared_ptr<AST::Node> Parser::branching() {
    if (speculate(&Parser::branchingIf, Parser::Rule::BranchingIf)) {
        return branchingIf();
    }

    if (speculate(&Parser::branchingWhile, Parser::Rule::BranchingWhile)) {
        return branchingWhile();
    }

    fail("Could not parse branching");
}

std::shared_ptr<AST::If> Parser::branchingIf() {
    consumeOrFail(Token::Type::If, "if");
    consumeOrFail('(', "(");
    doOrFail(speculate(&Parser::expression, Parser::Rule::Expression),
             "expression");
    auto condition = expression();
    consumeOrFail(')', ")");

    doOrFailMessage(speculate(&Parser::statement, Parser::Rule::Statement),
                    "statement", "'if' needs to be followed by a statement");
    auto positive = statement();
    decltype(statement()) negative{nullptr};

    if (consume(Token::Type::Else)) {
        doOrFailMessage(speculate(&Parser::statement, Parser::Rule::Statement),
                        "statement",
                        "'else' needs to be followed by a statement");
        negative = statement();
    }

    return std::make_shared<AST::If>(condition, positive, negative);
}

std::shared_ptr<AST::While> Parser::branchingWhile() {
    consumeOrFail(Token::Type::While, "while");
    consumeOrFail('(', "(");
    doOrFail(speculate(&Parser::expression, Parser::Rule::Expression),
             "expression");
    auto condition = expression();
    consumeOrFail(')', ")");

    doOrFailMessage(speculate(&Parser::statement, Parser::Rule::Statement),
                    "statement", "'while' needs to be followed by a statement");
    auto body = statement();

    return std::make_shared<AST::While>(condition, body);
}

std::shared_ptr<AST::Literal> Parser::boolean() {
    if (consume(Token::Type::True)) {
        return std::make_shared<AST::Literal>("true",
                                              DataType::Primitive::Bool);
    }

    if (consume(Token::Type::False)) {
        return std::make_shared<AST::Literal>("false",
                                              DataType::Primitive::Bool);
    }

    fail("Could not parse boolean, expecting 'true' or 'false'");
}

