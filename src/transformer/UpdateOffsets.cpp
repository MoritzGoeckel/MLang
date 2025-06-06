#include "UpdateOffsets.h"

#include "../error/Exceptions.h"

#include <algorithm>

void updateOffsets(CollectTypes::TypesMap& types) {
    for (auto& [name, type] : types) {
        if (type.isStruct()) {
            auto& structType = type.getStruct();
            size_t offset = 0;
            for (auto& [fieldName, field] : structType.fields) {
                if (field.offset == INVALID_OFFSET) {
                    field.offset = offset;
                    offset += field.type.getMemorySize();
                } else {
                    // If the offset is already set, we assume it is correct
                    // and do not change it.
                }
            }
        }
    }
}
