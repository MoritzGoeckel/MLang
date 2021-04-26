#pragma once

#include <string>
#include "GenerateMarkedCode.h"

class TypeError {
   private:
    std::string itsMessage;
    SourcePosition itsPosition;

   public:
    TypeError(const std::string& theMessage, const SourcePosition& thePosition);

    std::string generateString(const std::string& theSourceCode) const;
};
