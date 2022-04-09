#pragma once

#include <string>

class SourcePosition {
   public:
    SourcePosition();
    SourcePosition(const std::string& file, size_t line, size_t column);

    size_t getLine() const;
    size_t getColumn() const;
    const std::string& getFile() const;

    std::string toString() const;
    std::string toStandardString() const;

   private:
    std::string itsFile;
    size_t itsLine;
    size_t itsColumn;
};
