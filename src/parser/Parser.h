#pragma once

#include <deque>
#include <map>
#include <string>
#include <vector>
#include "../ast/Node.h"
#include "OperatorPrecedence.h"
#include "Tokenizer.h"

class Parser {
   public:
    Parser(std::vector<Token>&& tokens)
        : cache(),
          itsAst(nullptr),
          tokens(std::move(tokens)),
          markers(),
          idx(0u) {
        itsAst = file();
    }

    Parser(const std::vector<Token>& tokens)
        : cache(), itsAst(nullptr), tokens(tokens), markers(), idx(0u) {
        itsAst = file();
    }

    std::shared_ptr<AST::Node> getAst() { return itsAst; }

   private:
    enum class Rule {
        Expression,
        Statement,
        Ret,
        Block,
        NrExpression,
        InfixCall,
        Call,
        Literal,
        Assignment,
        ArgumentList,
        Identifier,
        Integer,
        Boolean,
        AssignmentLeft,
        VariableDecl,
        FunctionDecl,
        Branching,
        BranchingIf,
        BranchingWhile,
        ParenthesizedExpression
    };

    enum CacheResult { SUCCESS, FAILURE, MISS };

    CacheResult checkCache(size_t index, Rule rule) {
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

    template <typename Fn>
    bool speculate(Fn fn, Rule rule) {
        switch (checkCache(idx, rule)) {
            case CacheResult::FAILURE:
                return false;

            case CacheResult::SUCCESS:
                return true;

            case CacheResult::MISS:
                bool success = true;
                mark();
                std::shared_ptr<AST::Node> aResult = ((*this).*(fn))();
                if (!aResult) {
                    // Unsuccessful rules return nullptr
                    success = false;
                }
                reset();

                memorize(idx, rule, success ? SUCCESS : FAILURE);
                return success;
        }
        return false;
    }

    void memorize(size_t index, Rule rule, CacheResult result) {
        cache[index][rule] = result;  // Result could be stop index
    }

    void mark() { markers.push_back(idx); }

    void reset() {
        idx = markers.back();
        markers.pop_back();
    }

    Token lookAhead(size_t offset) {
        if (idx + offset >= tokens.size()) return Token();
        return tokens[idx + offset];
    }
    Token::Type lookAhead_t(size_t offset) {
        if (idx + offset >= tokens.size()) return Token::Type::None;
        return tokens[idx + offset].getType();
    }

    Token nextToken() { return lookAhead(0u); }

    bool isNext(Token::Type type) { return lookAhead_t(0) == type; }

    bool isNext(char c) {
        if (idx >= tokens.size()) return false;
        return tokens[idx].getChar() == c;
    }

    Token consume() { return tokens[idx++]; }

    bool consume(char c) {
        if (idx < tokens.size() && tokens[idx].getChar() == c) {
            ++idx;
            return true;
        } else {
            return false;
        }
    }

    bool consume(Token::Type expectedType) {
        if (idx < tokens.size() && tokens[idx].getType() == expectedType) {
            ++idx;
            return true;
        } else {
            return false;
        }
    }

    bool isDone() { return idx == tokens.size(); }

    std::vector<std::shared_ptr<AST::Node>> statementList() {
        std::vector<std::shared_ptr<AST::Node>> statements;
        while (speculate(&Parser::statement, Rule::Statement)) {
            statements.push_back(statement());
        }
        return statements;
    }

    std::shared_ptr<AST::Node> file() {
        auto statements = statementList();
        if (!isDone()) return nullptr;
        return std::make_shared<AST::Block>(statements);
    }

    std::shared_ptr<AST::Node> statement() {
        if (speculate(&Parser::ret, Rule::Ret)) {
            return ret();
        }

        if (speculate(&Parser::block, Rule::Block)) {
            return block();
        }

        if (speculate(&Parser::branching, Rule::Branching)) {
            return branching();
        }

        if (speculate(&Parser::expression, Rule::Expression)) {
            auto node = expression();
            if (!consume(Token::Type::StatementTerminator)) return nullptr;
            return node;
        }

        return nullptr;
    }

    std::shared_ptr<AST::Node> expression() {
        if (speculate(&Parser::infixCall, Rule::InfixCall)) {
            return infixCall();
        }

        if (speculate(&Parser::nrExpression, Rule::NrExpression)) {
            return nrExpression();
        }

        return nullptr;
    }

    std::shared_ptr<AST::Node> parenthesizedExpression() {
        if (!consume('(')) return nullptr;
        if (!speculate(&Parser::expression, Rule::Expression)) return nullptr;
        auto expr = expression();
        if (!expr) return nullptr;
        if (!consume(')')) return nullptr;
        return expr;
    }

    std::shared_ptr<AST::Node> nrExpression() {
        if (speculate(&Parser::parenthesizedExpression,
                      Rule::ParenthesizedExpression)) {
            return parenthesizedExpression();
        }

        // assignment
        if (speculate(&Parser::assignment, Rule::Assignment)) {
            return assignment();
        }

        // block
        if (speculate(&Parser::block, Rule::Block)) {
            return block();
        }

        // call
        if (speculate(&Parser::call, Rule::Call)) {
            return call();
        }

        // literal
        if (speculate(&Parser::literal, Rule::Literal)) {
            return literal();
        }

        // identifier
        if (speculate(&Parser::identifier, Rule::Identifier)) {
            return identifier();
        }
        return nullptr;
    }

    std::shared_ptr<AST::Ret> ret() {
        if (!consume(Token::Type::Ret)) return nullptr;

        if (consume(Token::Type::StatementTerminator)) {
            return std::make_shared<AST::Ret>();
        }

        if (!speculate(&Parser::expression, Rule::Expression)) return nullptr;
        auto expr = expression();
        if (!expr) return nullptr;

        if (!consume(Token::Type::StatementTerminator)) return nullptr;

        return std::make_shared<AST::Ret>(expr);
    }

    std::shared_ptr<AST::Block> block() {
        if (!consume('{')) return nullptr;
        auto statements = statementList();
        if (!consume('}')) return nullptr;
        return std::make_shared<AST::Block>(statements);
    }

    std::shared_ptr<AST::Call> call() {
        std::shared_ptr<AST::Identifier> method;
        std::vector<std::shared_ptr<AST::Node>> arguments;

        if (!speculate(&Parser::identifier, Rule::Identifier)) {
            return nullptr;
        }

        method = identifier();
        if (!method) return nullptr;

        if (!consume('(')) return nullptr;
        if (!argumentList(arguments)) return nullptr;
        if (!consume(')')) return nullptr;

        return std::make_shared<AST::Call>(method, arguments);
    }

    bool argumentList(std::vector<std::shared_ptr<AST::Node>>& theList) {
        do {
            if (!speculate(&Parser::expression, Rule::Expression)) return false;

            auto expr = expression();
            if (!expr) return false;
            theList.push_back(expr);
        } while (consume(Token::Type::Comma));
        return true;
    }

    // TODO: Identifier as args
    bool identifierList(std::vector<std::shared_ptr<AST::Node>>& theList) {
        do {
            if (speculate(&Parser::identifier, Rule::Identifier)) {
                auto ident = identifier();
                if (!ident) return false;
                theList.push_back(ident);  // TODO move
            } else {
                return false;
            }
        } while (consume(Token::Type::Comma));
        return true;
    }

    std::shared_ptr<AST::Identifier> identifier() {
        if (!(isNext(Token::Type::Identifier) || isNext(Token::Type::Special)))
            return nullptr;
        auto token = consume();
        return std::make_shared<AST::Identifier>(token.getContent());
    }

    std::shared_ptr<AST::Literal> literal() {
        if (speculate(&Parser::integer, Rule::Integer)) {
            return integer();
        }

        if (speculate(&Parser::boolean, Rule::Boolean)) {
            return boolean();
        }

        return nullptr;
    }

    std::shared_ptr<AST::Literal> integer() {
        if (!isNext(Token::Type::Number)) return nullptr;
        auto token = consume();

        return std::make_shared<AST::Literal>(token.getContent(),
                                              DataType::Primitive::Int);
    }

    std::shared_ptr<AST::Call> infixCall() {
        // Create list of operands and operators
        bool shouldBeExpression = true;

        std::deque<std::shared_ptr<AST::Identifier>> operators;
        std::deque<std::shared_ptr<AST::Node>> operands;

        while ((speculate(&Parser::nrExpression, Rule::NrExpression) &&
                shouldBeExpression) ||
               (speculate(&Parser::identifier, Rule::Identifier) &&
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

    std::shared_ptr<AST::Assign> assignment() {
        if (!speculate(&Parser::assignmentLeft, Rule::AssignmentLeft))
            return nullptr;
        auto left = assignmentLeft();

        if (!consume(Token::Type::Assignment)) return nullptr;

        if (!speculate(&Parser::expression, Rule::Expression)) return nullptr;
        auto right = expression();

        return std::make_shared<AST::Assign>(left, right);
    }

    std::shared_ptr<AST::Node> assignmentLeft() {
        if (speculate(&Parser::functionDecl, Rule::FunctionDecl)) {
            return functionDecl();
        }

        if (speculate(&Parser::variableDecl, Rule::VariableDecl)) {
            return variableDecl();
        }

        if (speculate(&Parser::identifier, Rule::Identifier)) {
            return identifier();
        }

        return nullptr;
    }

    std::shared_ptr<AST::Declvar> variableDecl() {
        if (!consume(Token::Type::Let)) return nullptr;
        if (!speculate(&Parser::identifier, Rule::Identifier)) return nullptr;
        return std::make_shared<AST::Declvar>(identifier());
    }

    // function_decl : 'let' identifier '(' identifier_list ? ')';
    std::shared_ptr<AST::Declfn> functionDecl() {
        if (!consume(Token::Type::Let)) return nullptr;
        if (!speculate(&Parser::identifier, Rule::Identifier)) return nullptr;
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

    std::shared_ptr<AST::Node> branching() {
        if (speculate(&Parser::branchingIf, Rule::BranchingIf)) {
            return branchingIf();
        }

        if (speculate(&Parser::branchingWhile, Rule::BranchingWhile)) {
            return branchingWhile();
        }

        return nullptr;
    }

    // 'if' '(' expr ')' positive ('else' negative)?
    std::shared_ptr<AST::If> branchingIf() {
        if (!consume(Token::Type::If)) return nullptr;
        if (!consume('(')) return nullptr;
        if (!speculate(&Parser::expression, Rule::Expression)) return nullptr;
        auto condition = expression();
        if (!consume(')')) return nullptr;

        if (!speculate(&Parser::statement, Rule::Statement)) return nullptr;
        auto positive = statement();
        decltype(statement()) negative{nullptr};

        if (consume(Token::Type::Else)) {
            if (!speculate(&Parser::statement, Rule::Statement)) return nullptr;
            negative = statement();
        }

        return std::make_shared<AST::If>(condition, positive, negative);
    }

    // 'while' '(' expr ')' statement ;
    std::shared_ptr<AST::While> branchingWhile() {
        if (!consume(Token::Type::While)) return nullptr;
        if (!consume('(')) return nullptr;
        if (!speculate(&Parser::expression, Rule::Expression)) return nullptr;
        auto condition = expression();
        if (!consume(')')) return nullptr;

        if (!speculate(&Parser::statement, Rule::Statement)) return nullptr;
        auto body = statement();

        return std::make_shared<AST::While>(condition, body);
    }

    std::shared_ptr<AST::Literal> boolean() {
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

    // [rule][idx] -> CacheResult
    std::map<size_t, std::map<Rule, CacheResult>> cache;
    std::shared_ptr<AST::Node> itsAst;
    std::vector<Token> tokens;
    std::vector<size_t> markers;
    size_t idx;
};

// Grammar for reference
/*
file                        : statements*
statement                   : ((expr | ret ) ';') | branching | block;
seperate by \n

expr                        : nr_expr               |
                              infix_call            ;

nr_expr                     : '(' expr ')'          |
                              assignment            |
                              block                 |
                              call                  |
                              literal               |
                              identifier            ;

ret                         : 'ret' expr ;

branching                   : branching_if | branching_while;

branching_if                : 'if' '(' expr ')' positive ('else' negative)? ;
positive                    : statement ;
negative                    : statement ;

branching_while             : 'while' '(' expr ')' statement ;

assignment                  : assignment_left '=' assignment_right ;
assignment_left             : identifier    |
                              variable_decl |
                              function_decl ;
assignment_right            : expr;

variable_decl               : 'let' identifier ;

identifier_list             : (identifier ',')* identifier ;
function_decl               : 'let' identifier '(' identifier_list? ')' ;

infix_call                  : infix_call_left identifier infix_call_right ;
infix_call_left             : nr_expr ;
infix_call_right            : expr ;

argument_list               : (expr ',')* expr ;
call                        : identifier '(' argument_list? ')' ;

block                       : '{' (statement | COMMENT)* '}' ;

literal                     : type_float  |
                              type_int    |
                              type_bool   |
                              type_string ;

type_float                  : FLOAT ;
type_int                    : INTEGER ;
type_bool                   : BOOL ;
type_string                 : STRING ;

FLOAT                       : [0-9]+ '.' [0-9]+ ;
INTEGER                     : [0-9]+ ;
STRING                      : '"' [a-zA-Z0-9]* '"' ;
BOOL                        : 'true' | 'false' ;

identifier                  : IDENTIFIER ;
IDENTIFIER                  : Alpha+(Numeric|Alpha)* ;
*/

