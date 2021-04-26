#include "ParseError.h"

#include "../error/GenerateMarkedCode.h"

ParseError::ParseError() : idx(0u), expected(), msg() {}

ParseError::ParseError(size_t idx, const std::string& expected,
                       const std::string& msg)
    : idx(idx), expected(expected), msg(msg) {}

size_t ParseError::getIndex() { return idx; }

std::string ParseError::getErrorMessage(const std::vector<Token>& tokens,
                                        const std::string& code) {
    // Show expected and found
    std::string errorMsg = "Expecting '" + expected + "' but found '" +
                           tokens[idx].getContent() + "'";

    // Show line number and column
    const auto& lastTokenPosition =
        idx > 0u ? tokens[idx - 1u].getPosition() : tokens[idx].getPosition();
    errorMsg += " @" + lastTokenPosition.toString();

    // New paragraph
    errorMsg += '\r';
    errorMsg += '\r';
    errorMsg += generateMarkedCode(lastTokenPosition, code);

    // Show message if available
    if (!msg.empty()) errorMsg += "\n" + msg + "\n";

    return errorMsg;
}
