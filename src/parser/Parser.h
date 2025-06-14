#pragma once

#include <map>
#include <string>
#include <vector>
#include "../ast/Node.h"
#include "ParseError.h"
#include "Tokenizer.h"

class Parser {
   public:
    Parser(std::vector<Token>&& tokens);
    Parser(const std::vector<Token>& tokens);

    std::shared_ptr<AST::Node> getAst();

    std::string getError();
    std::string getError(const std::string& sourceCode);

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
        LeftHandValue,
        VariableDecl,
        FunctionDecl,
        Branching,
        BranchingIf,
        BranchingWhile,
        ParenthesizedExpression,
        DeclStruct,
        TypeAnnotation,
        UninitializedVarDecl,
        StructAccess,
        ExternFn
    };

    enum CacheResult { SUCCESS, FAILURE, MISS };

    CacheResult checkCache(size_t index, Rule rule);

    template <typename Fn>
    bool speculate(Fn fn, Rule rule) {
        switch (checkCache(idx, rule)) {
            case CacheResult::FAILURE: {
                return false;
            }
            case CacheResult::SUCCESS: {
                return true;
            }
            case CacheResult::MISS: {
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
        }
        return false;
    }

    void memorize(size_t index, Rule rule, CacheResult result);
    void mark();
    void reset();

    Token lookAhead(size_t offset);
    Token::Type lookAhead_t(size_t offset);

    Token lastToken();
    Token nextToken();
    Token previousToken();
    bool isNext(Token::Type type);
    bool isNext(char c);

    const SourcePosition& getPosition();

    Token consume();
    bool consume(char c);
    bool consume(Token::Type expectedType);

    void report(std::string msg);
    void report(std::string expected, std::string msg);

    bool isDone();

    bool argumentList(std::vector<std::shared_ptr<AST::Node>>& theList);
    bool identifierList(std::vector<std::shared_ptr<AST::Identifier>>& theList);

    std::vector<std::shared_ptr<AST::Node>> statementList();

    std::shared_ptr<AST::Node> file();
    std::shared_ptr<AST::Node> statement();
    std::shared_ptr<AST::Node> expression();
    std::shared_ptr<AST::Node> parenthesizedExpression();
    std::shared_ptr<AST::Node> nrExpression();
    std::shared_ptr<AST::Ret> ret();
    std::shared_ptr<AST::ExternFn> externFn();
    std::shared_ptr<AST::Block> block();
    std::shared_ptr<AST::Call> call();
    std::shared_ptr<AST::Identifier> identifier();
    std::shared_ptr<AST::Literal> literal();
    std::shared_ptr<AST::Literal> integer();
    std::shared_ptr<AST::Call> infixCall();
    std::shared_ptr<AST::Assign> assignment();
    std::shared_ptr<AST::Node> leftHandValue();
    std::shared_ptr<AST::Declvar> variableDecl();
    std::shared_ptr<AST::Declfn> functionDecl();
    std::shared_ptr<AST::Node> branching();
    std::shared_ptr<AST::If> branchingIf();
    std::shared_ptr<AST::While> branchingWhile();
    std::shared_ptr<AST::Literal> boolean();
    std::shared_ptr<AST::DeclStruct> declStruct();
    std::shared_ptr<AST::Identifier> typeAnnotation();
    std::shared_ptr<AST::Declvar> uninitializedVarDecl();
    std::shared_ptr<AST::StructAccess> structAccess();

    std::map<size_t /*idx*/, std::map<Rule, CacheResult>> cache;

    std::shared_ptr<AST::Node> itsAst;
    ParseError itsError;

    std::vector<Token> tokens;
    std::vector<size_t> markers;
    size_t idx;
};
