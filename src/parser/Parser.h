#pragma once

#include <map>
#include <string>
#include <vector>
#include "../ast/Node.h"
#include "Tokenizer.h"

class Parser {
   public:
    Parser(std::vector<Token>&& tokens);
    Parser(const std::vector<Token>& tokens);

    std::shared_ptr<AST::Node> getAst();

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

    CacheResult checkCache(size_t index, Rule rule);

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

    void memorize(size_t index, Rule rule, CacheResult result);
    void mark();
    void reset();

    Token lookAhead(size_t offset);
    Token::Type lookAhead_t(size_t offset);

    Token nextToken();
    bool isNext(Token::Type type);
    bool isNext(char c);

    Token consume();
    bool consume(char c);
    bool consume(Token::Type expectedType);

    bool isDone();

    bool argumentList(std::vector<std::shared_ptr<AST::Node>>& theList);
    bool identifierList(std::vector<std::shared_ptr<AST::Node>>& theList);

    std::vector<std::shared_ptr<AST::Node>> statementList();

    std::shared_ptr<AST::Node> file();
    std::shared_ptr<AST::Node> statement();
    std::shared_ptr<AST::Node> expression();
    std::shared_ptr<AST::Node> parenthesizedExpression();
    std::shared_ptr<AST::Node> nrExpression();
    std::shared_ptr<AST::Ret> ret();
    std::shared_ptr<AST::Block> block();
    std::shared_ptr<AST::Call> call();
    std::shared_ptr<AST::Identifier> identifier();
    std::shared_ptr<AST::Literal> literal();
    std::shared_ptr<AST::Literal> integer();
    std::shared_ptr<AST::Call> infixCall();
    std::shared_ptr<AST::Assign> assignment();
    std::shared_ptr<AST::Node> assignmentLeft();
    std::shared_ptr<AST::Declvar> variableDecl();
    std::shared_ptr<AST::Declfn> functionDecl();
    std::shared_ptr<AST::Node> branching();
    std::shared_ptr<AST::If> branchingIf();
    std::shared_ptr<AST::While> branchingWhile();
    std::shared_ptr<AST::Literal> boolean();

    std::map<size_t /*idx*/, std::map<Rule, CacheResult>> cache;
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

