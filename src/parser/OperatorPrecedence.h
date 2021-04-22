#pragma once

#include "../ast/Node.h"

int precedence(std::shared_ptr<AST::Identifier> theOperator);
