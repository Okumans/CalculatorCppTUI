#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NODEPOINTER
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NODEPOINTER 

#include "unordered_map"
#include "runtimeTypedExprComponent.h"
#include "runtimeType.h"

inline NodePointer::NodePointer(NodePos target) :
	BaseRuntimeTypedExprComponent(
		RuntimeBaseType::NodePointer,
		target
	)
{}

inline NodePointer::NodePointer() :
	BaseRuntimeTypedExprComponent(
		RuntimeBaseType::NodePointer,
		NodeFactory::NodePosNull
	)
{}

inline bool NodePointer::isTypeValid(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunction) const {
	if (Result<RuntimeType, std::runtime_error> targetType{ getReturnType(mNodeExpression, EvaluatorLambdaFunction) }; targetType.isError())
		return false;
	return true;
}

inline bool NodePointer::isNodePointerValid() const {
	return NodeFactory::validNode(mNodeExpression);
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> NodePointer::getPointed(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunction) const {
	return RuntimeTypedExprComponent::fromNodeExpression(mNodeExpression, EvaluatorLambdaFunction);
}

inline NodeFactory::NodePos NodePointer::getPointerIndex() const {
	return mNodeExpression;
}

inline const NodeFactory::Node& NodePointer::getPointerNode() const {
	return NodeFactory::node(mNodeExpression);
}

inline void NodePointer::changePoint(NodePos target) {
	mNodeExpression = target;
}


inline std::string NodePointer::toString() const {
	return std::to_string(mNodeExpression);
}

inline NodeFactory::NodePos NodePointer::generateExpressionTree() const {
	return mNodeExpression; // always valid.
}



#endif // RUNTIME_TYPED_EXPR_COMPONENT_IMPL_NODEPOINTER
