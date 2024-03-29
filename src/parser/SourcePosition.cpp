#include "SourcePosition.h"

SourcePosition::SourcePosition() : itsFile{}, itsLine(0u), itsColumn(0u) {}

SourcePosition::SourcePosition(const std::string& file, size_t line,
                               size_t column)
    : itsFile(file), itsLine(line), itsColumn(column) {}

const std::string& SourcePosition::getFile() const { return itsFile; }
size_t SourcePosition::getLine() const { return itsLine; }
size_t SourcePosition::getColumn() const { return itsColumn; }

std::string SourcePosition::toString() const {
    return std::to_string(itsLine + 1u) + ":" + std::to_string(itsColumn + 1u);
}

std::string SourcePosition::toStandardString() const {
    return itsFile + ":" + std::to_string(itsLine + 1u) + ":" +
           std::to_string(itsColumn + 1u);
}
