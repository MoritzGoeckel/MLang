#pragma once

#include <string>

class SourcePosition {
   public:
    SourcePosition();
    SourcePosition(size_t line, size_t column);

    size_t getLine() const;
    size_t getColumn() const;
    std::string toString() const;

   private:
    size_t itsLine;
    size_t itsColumn;
};
