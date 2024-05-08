#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NUMBER
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NUMBER

#include "runtimeTypedExprComponent.h"

inline Number::Number(long double number) :
	BaseRuntimeTypedExprComponent{
		RuntimeBaseType::Number,
		generateExpressionTree(number)
	},
	mNumber{ number } {}

inline Number Number::fromExpressionNode(NodePos numberExpression) {
	return Number(numberExpression, true);
}

inline long double Number::getNumber() const {
	return mNumber;
}

inline Number::operator long double() const {
	return mNumber;
}

inline Number::Number(NodePos numberExpression, bool) :
	BaseRuntimeTypedExprComponent{
		RuntimeBaseType::Number,
		numberExpression
	},
	mNumber{ std::stold(NodeFactory::node(numberExpression).value) } {}

inline NodeFactory::NodePos Number::generateExpressionTree(long double number) const {
	return NodeFactory::create(std::to_string(number));
}

inline std::string Number::toString() const {
	return std::to_string(mNumber);
}

#endif //RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NUMBER