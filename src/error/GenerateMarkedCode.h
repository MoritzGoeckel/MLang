#pragma once

#include <string>
#include "../parser/SourcePosition.h"

std::string generateMarkedCode(const SourcePosition& thePosition,
                               const std::string& theCode);

