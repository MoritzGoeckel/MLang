#include "CollectTypes.h"

#include "../error/Exceptions.h"

#include <algorithm>

CollectTypes::CollectTypes(TypesMap& types) : types{types} {}

std::shared_ptr<AST::Node> CollectTypes::process(std::shared_ptr<AST::Node> node) {
    // TODO: Have another walker check for duplicate struct names
    if (node->getType() == AST::NodeType::DeclStruct) {
        auto declStruct = std::dynamic_pointer_cast<AST::DeclStruct>(node);
        const auto& structName = declStruct->getIdentifier()->getName();
        if(types.find(structName) == types.end()) {
            bool isComplete = true;
            std::map<std::string, StructMember> fields;
            for(const auto& aMember : declStruct->getMembers())
            {
                const auto& aMemberIdentifier = aMember->getIdentifier();
                ASSURE_NOT_NULL(aMemberIdentifier);
                const auto& aMemberName = aMemberIdentifier->getName();
                const auto& aMemberType = aMemberIdentifier->getDataType();
                if(aMemberType == DataType::Primitive::Unknown || aMemberType == DataType::Primitive::None) {
                    isComplete = false;
                    break;
                }
                fields.emplace(aMemberName, StructMember{aMemberType, INVALID_OFFSET});
            }
            if(isComplete) {
                auto structType = DataType::Struct{structName, fields}; // MGDO this should not work
                types.emplace(structType.name, structType);
            }
        }
    }
    followChildren(node);
    return node;
}
