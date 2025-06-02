#include "CollectTypes.h"

#include "../error/Exceptions.h"

#include <algorithm>

CollectTypes::CollectTypes() {}

std::shared_ptr<AST::Node> CollectTypes::process(std::shared_ptr<AST::Node> node) {
    if (node->getType() == AST::NodeType::DeclStruct) {
        auto declStruct = std::dynamic_pointer_cast<AST::DeclStruct>(node);
        std::vector<DataType> fields;
        for(const auto& aMember : declStruct->getMembers()){
            fields.push_back(aMember->getIdentifier()->getDataType());
        }
        auto structType = DataType::Struct{declStruct->getIdentifier()->getName(), fields};
        auto [it, inserted] = types.emplace(structType.name, structType);
        if (!inserted) {
            // TODO: Add message to error
            throwConstraintViolated("Duplicate struct declaration");
        }
    }
    followChildren(node);
    return node;
}
