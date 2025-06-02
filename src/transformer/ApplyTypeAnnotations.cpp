#include "ApplyTypeAnnotations.h"

#include "../error/Exceptions.h"

#include <algorithm>

ApplyTypeAnnotations::ApplyTypeAnnotations() {}

DataType typeFromString(const std::string& typeStr) {
    DataType::Primitive primitive = DataType::toPrimitive(typeStr);
    if(primitive != DataType::Primitive::Unknown) {
        return DataType(primitive);
    }

    return DataType(DataType::Struct{typeStr});
}

std::shared_ptr<AST::Node> ApplyTypeAnnotations::process(std::shared_ptr<AST::Node> node) {
    if (node->getType() == AST::NodeType::Declvar) {
        auto declvar = std::dynamic_pointer_cast<AST::Declvar>(node);
        if (declvar->hasTypeAnnotation()) {

            auto type = typeFromString(declvar->getTypeAnnotation());
            ASSURE(type != DataType::Primitive::Unknown, "Unknown type annotation");

            if(type == DataType::Primitive::Unknown) {
                this->addMessage("Bad type annotation: " + declvar->getTypeAnnotation());
            }  else {
                auto identifier = declvar->getIdentifier();
                ASSURE_NOT_NULL(identifier);
                identifier->setDataType(type, [this](auto& s) { this->addMessage(s); });
            }
        }
    }  else {
        followChildren(node);
    }

    return node;
}
