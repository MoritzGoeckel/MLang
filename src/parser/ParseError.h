#pragma once

#include <string>
#include <vector>
#include "Tokenizer.h"

class ParseError {
   public:
    ParseError();

    ParseError(size_t idx, const std::string& expected, const std::string& msg);

    std::string getErrorMessage(const std::vector<Token>& tokens,
                                const std::string& code);

    size_t getIndex();

   private:
    size_t idx;
    std::string expected;
    std::string msg;
};
