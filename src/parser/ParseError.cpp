#include "ParseError.h"

ParseError::ParseError() : idx(0u), expected(), msg() {}

ParseError::ParseError(size_t idx, const std::string& expected,
                       const std::string& msg)
    : idx(idx), expected(expected), msg(msg) {}

size_t ParseError::getIndex() { return idx; }

std::string ParseError::getErrorMessage(const std::vector<Token>& tokens,
                                        const std::string& code) {
    // Show expected and found
    std::string errorMsg = "Expected '" + expected + "' but found '" +
                           tokens[idx].getContent() + "'";

    // Show line number and column
    const auto& lastToken = idx > 0u ? tokens[idx - 1u] : tokens[idx];
    errorMsg += " @" + std::to_string(lastToken.getLine() + 1u) + ":" +
                std::to_string(lastToken.getColumn() + 1u);

    // Show faulty part of code if available
    if (!code.empty()) {
        // New paragraph
        errorMsg += '\n';
        errorMsg += '\n';

        size_t line = 0u;
        size_t i = 0u;

        // Line of token or one line before
        size_t targetLine = lastToken.getLine();
        if (targetLine > 0) targetLine -= 1u;

        // Seek until the line before the token
        while (i < code.size() && line < targetLine) {
            if (code[i] == '\n') line++;
            ++i;
        }

        // Print code & line numbers
        errorMsg += std::to_string(line) + ":  ";
        while (i < code.size() && line <= lastToken.getLine()) {
            errorMsg += code[i];  // Print code
            if (code[i] == '\n') {
                line++;
                // Print line number
                errorMsg += std::to_string(line) + ":  ";
            }
            ++i;
        }

        // Print pointer to previous token
        for (size_t i = 0; i < lastToken.getColumn(); ++i) errorMsg += ' ';
        errorMsg += '^';
        errorMsg += '\n';
    }

    // Show message if available
    if (!msg.empty()) errorMsg += "\n" + msg + "\n";

    return errorMsg;
}
