#include "ApplyTypeAnnotations.h"

#include "../error/Exceptions.h"

#include <algorithm>

ApplyTypeAnnotations::ApplyTypeAnnotations(CollectTypes::TypesMap& types) : types{types} {}

std::shared_ptr<AST::Node> ApplyTypeAnnotations::process(std::shared_ptr<AST::Node> node) {
    if (node->getType() == AST::NodeType::Declvar) {
        auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
        if (declvar->hasTypeAnnotation()) {
            const auto& annotationText = declvar->getTypeAnnotation();
            auto identifier = declvar->getIdentifier();
            ASSURE_NOT_NULL(identifier);

            if(types.find(annotationText) != types.end()) {
                identifier->setDataType(types[annotationText], [this](auto& s) { this->addMessage(s); });
            } else {
                DataType::Primitive primitive = DataType::toPrimitive(annotationText);
                if(primitive != DataType::Primitive::Unknown) {
                    identifier->setDataType(DataType(primitive), [this](auto& s) { this->addMessage(s); });
                }
            }
        }
    }  else {
        followChildren(node);
    }

    return node;
}
