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
    if (!isDone()) return nullptr;
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
        if (!consume(Token::Type::StatementTerminator)) return nullptr;
        return node;
    }

    return nullptr;
}

std::shared_ptr<AST::Node> Parser::expression() {
    if (speculate(&Parser::infixCall, Parser::Rule::InfixCall)) {
        return infixCall();
    }

    if (speculate(&Parser::nrExpression, Parser::Rule::NrExpression)) {
        return nrExpression();
    }

    return nullptr;
}

std::shared_ptr<AST::Node> Parser::parenthesizedExpression() {
    if (!consume('(')) return nullptr;
    if (!speculate(&Parser::expression, Parser::Rule::Expression))
        return nullptr;
    auto expr = expression();
    if (!expr) return nullptr;
    if (!consume(')')) return nullptr;
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
    return nullptr;
}

std::shared_ptr<AST::Ret> Parser::ret() {
    if (!consume(Token::Type::Ret)) return nullptr;

    if (consume(Token::Type::StatementTerminator)) {
        return std::make_shared<AST::Ret>();
    }

    if (!speculate(&Parser::expression, Parser::Rule::Expression))
        return nullptr;
    auto expr = expression();
    if (!expr) return nullptr;

    if (!consume(Token::Type::StatementTerminator)) return nullptr;

    return std::make_shared<AST::Ret>(expr);
}

std::shared_ptr<AST::Block> Parser::block() {
    if (!consume('{')) return nullptr;
    auto statements = statementList();
    if (!consume('}')) return nullptr;
    return std::make_shared<AST::Block>(statements);
}

std::shared_ptr<AST::Call> Parser::call() {
    std::shared_ptr<AST::Identifier> method;
    std::vector<std::shared_ptr<AST::Node>> arguments;

    if (!speculate(&Parser::identifier, Parser::Rule::Identifier)) {
        return nullptr;
    }

    method = identifier();
    if (!method) return nullptr;

    if (!consume('(')) return nullptr;
    if (!argumentList(arguments)) return nullptr;
    if (!consume(')')) return nullptr;

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
    if (!(isNext(Token::Type::Identifier) || isNext(Token::Type::Special)))
        return nullptr;
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

    return nullptr;
}

std::shared_ptr<AST::Literal> Parser::integer() {
    if (!isNext(Token::Type::Number)) return nullptr;
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
        operands.size() != operators.size() + 1u)
        return nullptr;

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

    if (operands.size() != 1u || !operators.empty()) return nullptr;

    return std::dynamic_pointer_cast<AST::Call>(operands.front());
}

std::shared_ptr<AST::Assign> Parser::assignment() {
    if (!speculate(&Parser::assignmentLeft, Parser::Rule::AssignmentLeft))
        return nullptr;
    auto left = assignmentLeft();

    if (!consume(Token::Type::Assignment)) return nullptr;

    if (!speculate(&Parser::expression, Parser::Rule::Expression))
        return nullptr;
    auto right = expression();

    return std::make_shared<AST::Assign>(left, right);
}

std::shared_ptr<AST::Node> Parser::assignmentLeft() {
    if (speculate(&Parser::functionDecl, Parser::Rule::FunctionDecl)) {
        return functionDecl();
    }

    if (speculate(&Parser::variableDecl, Parser::Rule::VariableDecl)) {
        return variableDecl();
    }

    if (speculate(&Parser::identifier, Parser::Rule::Identifier)) {
        return identifier();
    }

    return nullptr;
}

std::shared_ptr<AST::Declvar> Parser::variableDecl() {
    if (!consume(Token::Type::Let)) return nullptr;
    if (!speculate(&Parser::identifier, Parser::Rule::Identifier))
        return nullptr;
    return std::make_shared<AST::Declvar>(identifier());
}

std::shared_ptr<AST::Declfn> Parser::functionDecl() {
    if (!consume(Token::Type::Let)) return nullptr;
    if (!speculate(&Parser::identifier, Parser::Rule::Identifier))
        return nullptr;
    auto method = identifier();
    if (!consume('(')) return nullptr;
    std::vector<std::shared_ptr<AST::Node>> params;

    if (!isNext(')')) {
        // Identifier list is optional
        if (!identifierList(params)) return nullptr;
    }

    if (!consume(')')) return nullptr;

    return std::make_shared<AST::Declfn>(method, params);
}

std::shared_ptr<AST::Node> Parser::branching() {
    if (speculate(&Parser::branchingIf, Parser::Rule::BranchingIf)) {
        return branchingIf();
    }

    if (speculate(&Parser::branchingWhile, Parser::Rule::BranchingWhile)) {
        return branchingWhile();
    }

    return nullptr;
}

std::shared_ptr<AST::If> Parser::branchingIf() {
    if (!consume(Token::Type::If)) return nullptr;
    if (!consume('(')) return nullptr;
    if (!speculate(&Parser::expression, Parser::Rule::Expression))
        return nullptr;
    auto condition = expression();
    if (!consume(')')) return nullptr;

    if (!speculate(&Parser::statement, Parser::Rule::Statement)) return nullptr;
    auto positive = statement();
    decltype(statement()) negative{nullptr};

    if (consume(Token::Type::Else)) {
        if (!speculate(&Parser::statement, Parser::Rule::Statement))
            return nullptr;
        negative = statement();
    }

    return std::make_shared<AST::If>(condition, positive, negative);
}

std::shared_ptr<AST::While> Parser::branchingWhile() {
    if (!consume(Token::Type::While)) return nullptr;
    if (!consume('(')) return nullptr;
    if (!speculate(&Parser::expression, Parser::Rule::Expression))
        return nullptr;
    auto condition = expression();
    if (!consume(')')) return nullptr;

    if (!speculate(&Parser::statement, Parser::Rule::Statement)) return nullptr;
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
    return nullptr;
}

