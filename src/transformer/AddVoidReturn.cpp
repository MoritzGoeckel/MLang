#include "AddVoidReturn.h"

#include "../error/Exceptions.h"
#include <iostream>

namespace transformer {
AddVoidReturn::AddVoidReturn() {}

std::shared_ptr<AST::Function> AddVoidReturn::process(const std::shared_ptr<AST::Function>& node) {
	const auto& returnType = node->getDataType().getReturn();
	if(returnType && *returnType == DataType::Primitive::Void) {
		// We only consider functions that return void
		return node;
	}

	auto body = node->getBody();
	if(!body) { throwConstraintViolated("Body is nullptr"); }

	if(body->getType() != AST::NodeType::Block) {
		std::cout << __FILE__ << __LINE__ << std::endl;
		if (body->getType() == AST::NodeType::Ret) {
		return node; // Already has a return statement
		}
		auto block = std::make_shared<AST::Block>(body->getPosition());
		block->addChild(body);
		block->addChild(std::make_shared<AST::Ret>(body->getPosition()));
		node->setBody(block);
	} else {

		auto block = std::dynamic_pointer_cast<AST::Block>(body);
		if (block->getChildren().back()->getType() == AST::NodeType::Ret){
		return node; // Already has a return statement
		}
		block->addChild(std::make_shared<AST::Ret>(body->getPosition()));
	}
	return node;
}

} // namespace transformer
