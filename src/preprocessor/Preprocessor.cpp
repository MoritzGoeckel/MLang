#include "Preprocessor.h"

void Preprocessor::run(std::string& str) {
    // Add block
    str.insert(0, "{");
    str.append("}");
}
