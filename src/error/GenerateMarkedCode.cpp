#include "GenerateMarkedCode.h"

std::string generateMarkedCode(const SourcePosition& thePosition,
                               const std::string& theCode) {
    std::string errorMsg;

    // Show faulty part of code if available
    if (!theCode.empty()) {
        size_t line = 0u;
        size_t i = 0u;

        // Line of token or one line before
        size_t targetLine = thePosition.getLine();
        if (targetLine > 0) targetLine -= 1u;

        // Seek until the line before the token
        while (i < theCode.size() && line < targetLine) {
            if (theCode[i] == '\n') line++;
            ++i;
        }

        // Print code & line numbers
        errorMsg += std::to_string(line) + ":  ";
        while (i < theCode.size() && line <= thePosition.getLine()) {
            errorMsg += theCode[i];  // Print code
            if (theCode[i] == '\n') {
                line++;
                // Print line number
                errorMsg += std::to_string(line) + ":  ";
            }
            ++i;
        }

        // Print pointer to previous token
        for (size_t i = 0; i < thePosition.getColumn(); ++i) errorMsg += ' ';
        errorMsg += '^';
        errorMsg += '\n';
    }

    return errorMsg;
}
