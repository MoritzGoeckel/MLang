#include "TypeError.h"

TypeError::TypeError(const std::string& theMessage,
                     const SourcePosition& thePosition)
    : itsMessage(theMessage), itsPosition(thePosition) {}

std::string TypeError::generateString(const std::string& theSourceCode) const {
    std::string aResult;
    aResult += generateMarkedCode(itsPosition, theSourceCode);
    aResult += "\n" + itsMessage + "\n";
    return aResult;
}
