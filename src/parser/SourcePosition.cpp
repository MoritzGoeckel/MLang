#include "SourcePosition.h"

SourcePosition::SourcePosition() : itsLine(0u), itsColumn(0u) {}

SourcePosition::SourcePosition(size_t line, size_t column)
    : itsLine(line), itsColumn(column) {}

size_t SourcePosition::getLine() const { return itsLine; }
size_t SourcePosition::getColumn() const { return itsColumn; }

std::string SourcePosition::toString() const {
    return std::to_string(itsLine + 1u) + ":" + std::to_string(itsColumn + 1u);
}
