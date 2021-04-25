#include "Tokenizer.h"

#include <iostream>

namespace CharCategories {

bool isParen(char c) {
    return c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')';
    // Attention! < and > are not parenthesis
    //|| c == '<' || c == '>';
}
bool isNumeric(char c) { return c >= '0' && c <= '9'; }
bool isAlphabetic(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
bool isAlphanumeric(char c) { return isNumeric(c) || isAlphabetic(c); }
bool isSpecial(char c) {
    if (isParen(c)) return false;
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') ||
           (c >= '[' && c <= '_') || (c >= '{' && c <= '~');
}
bool isCommentStart(char c) { return c == '#'; }
bool isCommentTerminator(char c) { return c == '\n'; }
bool isStatementTerminator(char c) {
    return c == ';'; /* TODO maybe also new line */
}

}  // namespace CharCategories

// TODO Put token in its own file

Token::Token() : type(Token::Type::None), content(""), column(0u), line(0u) {}

Token::Token(const std::string& content, size_t line, size_t column)
    : type(Token::Type::None), content(content), line(line), column(column) {
    determineType();
}

Token::Token(std::string&& content, size_t line, size_t column)
    : type(Token::Type::None),
      content(std::move(content)),
      line(line),
      column(column) {
    determineType();
}

const std::string& Token::getContent() const { return content; }

char Token::getChar() {
    if (getContent().size() == 1u)
        return getContent()[0];
    else
        return static_cast<char>(0u);
}

Token::Type Token::getType() const { return type; }

bool Token::isTrivialContent() const {
    switch (type) {
        case Token::Type::None:
        case Token::Type::StatementTerminator:
        case Token::Type::Let:
        case Token::Type::Ret:
        case Token::Type::While:
        case Token::Type::If:
        case Token::Type::Else:
        case Token::Type::Assignment:
        case Token::Type::Comma:
        case Token::Type::True:
        case Token::Type::False:
            return true;
        default:
            return false;
    }
}

bool Token::handleIsKeyword() {
    if (content == "=") {
        type = Token::Type::Assignment;
        return true;
    }

    if (content == ",") {
        type = Token::Type::Comma;
        return true;
    }

    // Shortcut
    if (content.size() < 2) return false;

    if (content == "if") {
        type = Token::Type::If;
        return true;
    }

    // Shortcut
    if (content.size() < 3) return false;

    if (content == "let") {
        type = Token::Type::Let;
        return true;
    }

    if (content == "ret") {
        type = Token::Type::Ret;
        return true;
    }

    if (content == "true") {
        type = Token::Type::True;
        return true;
    }

    if (content == "false") {
        type = Token::Type::False;
        return true;
    }

    // Shortcut
    if (content.size() < 4) return false;

    if (content == "else") {
        type = Token::Type::Else;
        return true;
    }

    if (content == "while") {
        type = Token::Type::While;
        return true;
    }

    return false;
}

void Token::determineType() {
    if (content.size() == 1u && CharCategories::isParen(content[0])) {
        type = Token::Type::Parenthesis;
        return;
    }

    if (content.size() == 1u &&
        CharCategories::isStatementTerminator(content[0])) {
        type = Token::Type::StatementTerminator;
        return;
    }

    if (handleIsKeyword()) return;

    bool allNumber = true;
    for (char c : content) {
        if (!CharCategories::isNumeric(c)) allNumber = false;
        if (CharCategories::isSpecial(c)) {
            type = Token::Type::Special;
            return;
        }
    }
    if (allNumber) {
        type = Token::Type::Number;
        return;
    }

    type = Token::Type::Identifier;
}

size_t Token::getLine() const {
    // TODO Should not be type::none
    return line;
}

size_t Token::getColumn() const { return column; }

std::string to_string(Token::Type theType) {
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

std::ostream& operator<<(std::ostream& theStream, Token::Type theType) {
    return theStream << to_string(theType);
}

std::ostream& operator<<(std::ostream& theStream, const Token& theToken) {
    theStream << theToken.getType();
    if (!theToken.isTrivialContent())
        theStream << "(" << theToken.getContent() << ")";
    return theStream;
}

Tokenizer::Tokenizer(const std::string& theInput)
    : tokens(),
      buffer(),
      isAlphanumericBuffer(true),
      inComment(false),
      line(0u),
      column(0u),
      bufferStartLine(0u),
      bufferStartColumn(0u) {
    for (char c : theInput) {
        if (c == '\n') {
            ++line;
            column = 0u;
        } else {
            ++column;
        }

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
            tokens.emplace_back(std::string(1, c), line, column - 1u);
            continue;
        }

        // Anything unknown is a seperator
        if (!aSpecial && !aAlphanumeric) {
            pushBuffer();
            continue;
        }

        // Its either special or alphanumeric. Push.
        if (buffer.empty()) {
            bufferStartColumn = column;
            bufferStartLine = line;
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

const std::vector<Token>& Tokenizer::getTokens() const { return tokens; }

void Tokenizer::pushBuffer() {
    if (!buffer.empty()) {
        tokens.emplace_back(buffer, bufferStartLine, bufferStartColumn - 1u);
        buffer.clear();
    }
}

void Tokenizer::addToBuffer(char c) { buffer += c; }

