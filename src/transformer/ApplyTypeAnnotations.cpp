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
                } else {
                    std::string msg = "Invalid type annotation '" + annotationText +
                                     "' for variable '" + identifier->getName() + "'";
                    itsErrors.emplace_back(msg, declvar->getPosition());
                }
            }
        }
    } if (node->getType() == AST::NodeType::Identifier) {
        auto identifier = std::dynamic_pointer_cast<AST::Identifier>(node);
        if(identifier->hasTypeAnnotation()) {
            const auto& annotationText = identifier->getTypeAnnotation();
            if(types.find(annotationText) != types.end()) {
                identifier->setDataType(types[annotationText], [this](auto& s) { this->addMessage(s); });
            } else {
                DataType::Primitive primitive = DataType::toPrimitive(annotationText);
                if(primitive != DataType::Primitive::Unknown) {
                    identifier->setDataType(DataType(primitive), [this](auto& s) { this->addMessage(s); });
                } else {
                    std::string msg = "Invalid type annotation '" + annotationText +
                                     "' for identifier '" + identifier->getName() + "'";
                    itsErrors.emplace_back(msg, identifier->getPosition());
                }
            }
        }
    } if (node->getType() == AST::NodeType::ExternFn) {
        auto externFn = std::dynamic_pointer_cast<AST::ExternFn>(node);
        for(auto& param : externFn->getParameters()) {
            ASSURE_NOT_NULL(param);
            process(param);
        }

        std::shared_ptr<DataType> returnType{nullptr};
        if (externFn->hasTypeAnnotation()) {
            const auto& annotationText = externFn->getTypeAnnotation();
            if (types.find(annotationText) != types.end()) {
                returnType = std::make_shared<DataType>(types[annotationText]); // TODO: Should just return shared_ptr
            } else {
                DataType::Primitive primitive = DataType::toPrimitive(annotationText);
                if (primitive != DataType::Primitive::Unknown) {
                    returnType = std::make_shared<DataType>(primitive);
                } else {
                    std::string msg = "Invalid return type annotation '" + annotationText +
                                     "' for extern function";
                    itsErrors.emplace_back(msg, externFn->getPosition());
                }
            }
        }

        if(!returnType || *returnType == DataType::Primitive::Unknown) {
            return node;
        }

        std::vector<DataType> params;
        for (const auto& param : externFn->getParameters()) {
            ASSURE_NOT_NULL(param);
            const auto& type = param->getDataType();
            if (type == DataType::Primitive::Unknown) {
                return node;
            }
            params.push_back(type);
        }

        if(returnType) {
            externFn->setDataType(DataType(params, *returnType, true /* extern */), [this](auto& s) { this->addMessage(s); });
        }
    } else {
        followChildren(node);
    }

    return node;
}
