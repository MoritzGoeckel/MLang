#pragma once

#include <iostream>
#include <string>
#include <vector>

// TODO remove inline, create cpp

namespace CharCategories {

inline bool isParen(char c) {
    return c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')';
    // Attention! < and > are not parenthesis
    //|| c == '<' || c == '>';
}
inline bool isNumeric(char c) { return c >= '0' && c <= '9'; }
inline bool isAlphabetic(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
inline bool isAlphanumeric(char c) { return isNumeric(c) || isAlphabetic(c); }
inline bool isSpecial(char c) {
    if (isParen(c)) return false;
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') ||
           (c >= '[' && c <= '_') || (c >= '{' && c <= '~');
}
inline bool isCommentStart(char c) { return c == '#'; }
inline bool isCommentTerminator(char c) { return c == '\n'; }
inline bool isStatementTerminator(char c) {
    return c == ';'; /* TODO maybe also new line */
}

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

    Token() : type(Type::None), content("") {}

    Token(const std::string& content) : type(Type::None), content(content) {
        determineType();
    }

    Token(std::string&& content)
        : type(Type::None), content(std::move(content)) {
        determineType();
    }

    const std::string& getContent() const { return content; }

    char getChar() {
        if (getContent().size() == 1u)
            return getContent()[0];
        else
            return static_cast<char>(0u);
    }
    Type getType() const { return type; }

    bool isTrivialContent() const {
        switch (type) {
            case Type::None:
            case Type::StatementTerminator:
            case Type::Let:
            case Type::Ret:
            case Type::While:
            case Type::If:
            case Type::Else:
            case Type::Assignment:
            case Type::Comma:
            case Type::True:
            case Type::False:
                return true;
            default:
                return false;
        }
    }

   private:
    bool handleIsKeyword() {
        if (content == "=") {
            type = Type::Assignment;
            return true;
        }

        if (content == ",") {
            type = Type::Comma;
            return true;
        }

        // Shortcut
        if (content.size() < 2) return false;

        if (content == "if") {
            type = Type::If;
            return true;
        }

        // Shortcut
        if (content.size() < 3) return false;

        if (content == "let") {
            type = Type::Let;
            return true;
        }

        if (content == "ret") {
            type = Type::Ret;
            return true;
        }

        if (content == "true") {
            type = Type::True;
            return true;
        }

        if (content == "false") {
            type = Type::False;
            return true;
        }

        // Shortcut
        if (content.size() < 4) return false;

        if (content == "else") {
            type = Type::Else;
            return true;
        }

        if (content == "while") {
            type = Type::While;
            return true;
        }

        return false;
    }

    void determineType() {
        if (content.size() == 1u && CharCategories::isParen(content[0])) {
            type = Type::Parenthesis;
            return;
        }

        if (content.size() == 1u &&
            CharCategories::isStatementTerminator(content[0])) {
            type = Type::StatementTerminator;
            return;
        }

        if (handleIsKeyword()) return;

        bool allNumber = true;
        for (char c : content) {
            if (!CharCategories::isNumeric(c)) allNumber = false;
            if (CharCategories::isSpecial(c)) {
                type = Type::Special;
                return;
            }
        }
        if (allNumber) {
            type = Type::Number;
            return;
        }

        type = Type::Identifier;
    }

    Type type;
    std::string content;
};

inline std::string to_string(Token::Type theType) {
    switch (theType) {
        case Token::Type::Identifier:
            return "Identifier";
        case Token::Type::Special:
            return "Special";
        case Token::Type::Number:
            return "Number";
        case Token::Type::Parenthesis:
            return "Parenthesis";
        case Token::Type::None:
            return "None";
        case Token::Type::StatementTerminator:
            return "StatementTerminator";
        case Token::Type::Let:
            return "Let";
        case Token::Type::Ret:
            return "Ret";
        case Token::Type::While:
            return "While";
        case Token::Type::If:
            return "If";
        case Token::Type::Else:
            return "Else";
        case Token::Type::Assignment:
            return "Assignment";
        case Token::Type::Comma:
            return "Comma";
        case Token::Type::True:
            return "True";
        case Token::Type::False:
            return "False";
    }
}

inline std::ostream& operator<<(std::ostream& theStream, Token::Type theType) {
    return theStream << to_string(theType);
}

inline std::ostream& operator<<(std::ostream& theStream,
                                const Token& theToken) {
    theStream << theToken.getType();
    if (!theToken.isTrivialContent())
        theStream << "(" << theToken.getContent() << ")";
    return theStream;
}

class Tokenizer {
   public:
    Tokenizer(const std::string& theInput)
        : tokens(), buffer(), isAlphanumericBuffer(true), inComment(false) {
        for (char c : theInput) {
            if (CharCategories::isCommentStart(c)) {
                inComment = true;
                continue;
            }

            // Skip comments
            if (inComment) {
                if (CharCategories::isCommentTerminator(c)) inComment = false;
                continue;
            }

            bool aAlphanumeric = CharCategories::isAlphanumeric(c);
            bool aSpecial = CharCategories::isSpecial(c);

            // Parenthesis are their own tokens
            if (CharCategories::isParen(c)) {
                pushBuffer();
                tokens.push_back(std::string(1, c));
                continue;
            }

            // Anything unknown is a seperator
            if (!aSpecial && !aAlphanumeric) {
                pushBuffer();
                continue;
            }

            // Its either special or alphanumeric. Push.
            if (buffer.empty()) {
                addToBuffer(c);
                isAlphanumericBuffer = aAlphanumeric;
                continue;
            }

            if ((isAlphanumericBuffer && aAlphanumeric) ||
                (!isAlphanumericBuffer && aSpecial)) {
                addToBuffer(c);
            } else {
                // Change from alphanumeric to special. Push.
                pushBuffer();
                addToBuffer(c);
                isAlphanumericBuffer = aAlphanumeric;
            }
        }
        pushBuffer();
    }

    const std::vector<Token>& getTokens() { return tokens; }

   private:
    void pushBuffer() {
        if (!buffer.empty()) {
            tokens.push_back(Token(buffer));
            buffer.clear();
        }
    }

    void addToBuffer(char c) { buffer += c; }

    std::vector<Token> tokens;
    std::string buffer;
    bool isAlphanumericBuffer;
    bool inComment;
};

