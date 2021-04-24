#pragma once

#include <string>
#include <vector>

namespace CharCategories {

bool isParen(char c);
bool isNumeric(char c);
bool isAlphabetic(char c);
bool isAlphanumeric(char c);
bool isSpecial(char c);
bool isCommentStart(char c);
bool isCommentTerminator(char c);
bool isStatementTerminator(char c);

}  // namespace CharCategories

class Token {
   public:
    enum class Type {
        // Default
        None,
        // General
        Identifier,
        Special,
        Number,
        Parenthesis,
        StatementTerminator,
        Comma,
        // Keywords
        Let,
        Ret,
        If,
        Else,
        While,
        Assignment,
        True,
        False
    };
    // TODO: String literal

    Token();
    Token(const std::string& content);
    Token(std::string&& content);

    const std::string& getContent() const;
    char getChar();
    Type getType() const;
    bool isTrivialContent() const;

   private:
    bool handleIsKeyword();
    void determineType();

    Type type;
    std::string content;
};

std::string to_string(Token::Type theType);

std::ostream& operator<<(std::ostream& theStream, Token::Type theType);
std::ostream& operator<<(std::ostream& theStream, const Token& theToken);

class Tokenizer {
   public:
    Tokenizer(const std::string& theInput);

    const std::vector<Token>& getTokens() const;

   private:
    void pushBuffer();
    void addToBuffer(char c);

    std::vector<Token> tokens;
    std::string buffer;
    bool isAlphanumericBuffer;
    bool inComment;
};

