#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY

#include "runtimeTypedExprComponent.h"
#include <format>
#include <algorithm>

inline const Number& RuntimeTypedExprComponent::getNumber() const {
	return std::get<Number>(*this);
}

inline const Lambda& RuntimeTypedExprComponent::getLambda() const {
	return std::get<Lambda>(*this);
}

inline const Storage& RuntimeTypedExprComponent::getStorage() const {
	return std::get<Storage>(*this);
}

inline const NodePointer& RuntimeTypedExprComponent::getNodePointer() const {
	return std::get<NodePointer>(*this);
}

inline RuntimeBaseType RuntimeTypedExprComponent::getTypeHolded() const {
	return mStoredType;
}

inline RuntimeType RuntimeTypedExprComponent::getDetailTypeHold() const {
	return std::visit([](const auto& arg) -> RuntimeType {
		if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, Number> ||
			std::is_same_v<std::decay_t<decltype(arg)>, Storage> ||
			std::is_same_v<std::decay_t<decltype(arg)>, Lambda> ||
			std::is_same_v<std::decay_t<decltype(arg)>, NodePointer>) {
			return arg.getType();
		}
		unreachable();
		}, *this);
}

inline const RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator[](size_t index) const {
	if (getTypeHolded() != RuntimeBaseType::_Storage)
		throw std::runtime_error("Unable to access an element from a non-Storage type.");
	return getStorage()[index];
}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Storage& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(component),
	mStoredType{ RuntimeBaseType::_Storage } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Storage&& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(std::move(component)),
	mStoredType{ RuntimeBaseType::_Storage } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Lambda& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(component),
	mStoredType{ RuntimeBaseType::_Lambda } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Lambda&& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(std::move(component)),
	mStoredType{ RuntimeBaseType::_Lambda } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Number& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(component),
	mStoredType{ RuntimeBaseType::Number } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Number&& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(std::move(component)),
	mStoredType{ RuntimeBaseType::Number } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const NodePointer& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(component),
	mStoredType{ RuntimeBaseType::NodePointer } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(NodePointer&& component) :
	std::variant<Number, Storage, Lambda, NodePointer>(std::move(component)),
	mStoredType{ RuntimeBaseType::NodePointer } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(long double component) :
	std::variant<Number, Storage, Lambda, NodePointer>(Number(component)),
	mStoredType{ RuntimeBaseType::Number } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const RuntimeTypedExprComponent& other) :
	std::variant<Number, Storage, Lambda, NodePointer>(other),
	mStoredType{ other.mStoredType } {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(RuntimeTypedExprComponent&& other) noexcept :
	std::variant<Number, Storage, Lambda, NodePointer>(std::move(other)),
	mStoredType{ std::move(other.mStoredType) } {}

inline RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator=(const RuntimeTypedExprComponent& other) {
	if (this != &other) {
		mStoredType = other.mStoredType;
		std::variant<Number, Storage, Lambda, NodePointer>::operator=(other);
	}
	return *this;
}

inline RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator=(RuntimeTypedExprComponent&& other) noexcept {
	if (this != &other) {
		mStoredType = std::move(other.mStoredType);
		std::variant<Number, Storage, Lambda, NodePointer>::operator=(std::move(other));
	}
	return *this;
}

inline std::string RuntimeTypedExprComponent::toString() const {
	return std::visit([](const auto& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, Number> ||
			std::is_same_v<T, Storage> ||
			std::is_same_v<T, Lambda> ||
			std::is_same_v<T, NodePointer>) {
			return arg.toString();
		}
		throw std::runtime_error("Invalid type in variant.");
		}, *this);
}

inline NodeFactory::NodePos RuntimeTypedExprComponent::toNodeExpression() const {
	return std::visit([](const auto& arg) -> NodeFactory::NodePos {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, Number> ||
			std::is_same_v<T, Storage> ||
			std::is_same_v<T, Lambda> ||
			std::is_same_v<T, NodePointer>) {
			return arg.getNodeExpression();
		}
		throw std::runtime_error("Invalid type in variant. Btw how can it happen?");
		}, *this);
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> RuntimeTypedExprComponent::fromNodeExpression(NodeFactory::NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodeFactory::NodePos, NodeFactory::NodePos>& nodeDependency) {
	switch (NodeFactory::Node::NodeState currNodeState{ NodeFactory::node(rootNodeExpression).nodeState })
	{
	case NodeFactory::Node::NodeState::LambdaFuntion:
	{
		Result result = Lambda::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions);
		if (result.isError())
			return RuntimeError<RuntimeTypeError>(
				result.getException(),
				std::format("When trying to convert nodeExpression (value=\"{}\") to LambdaFunction.",
					NodeFactory::node(rootNodeExpression).value),
				"RuntimeTypedExprComponent::fromNodeExpression"
			);

		return RuntimeTypedExprComponent(result.getValue());
	}
	case NodeFactory::Node::NodeState::Operator:
	{
		Result result = Lambda::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions);
		if (result.isError())
			return RuntimeError<RuntimeTypeError>(
				result.getException(),
				std::format(
					"When trying to convert nodeExpression (value=\"{}\") to Operator.",
					NodeFactory::node(rootNodeExpression).value
				),
				"RuntimeTypedExprComponent::fromNodeExpression"
			);

		return RuntimeTypedExprComponent(result.getValue());
	}
	case NodeFactory::Node::NodeState::Number:
		return RuntimeTypedExprComponent(Number::fromExpressionNode(rootNodeExpression));

	case NodeFactory::Node::NodeState::Storage:
	{
		Result result = Storage::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions, nodeDependency);
		if (result.isError())
			return RuntimeError<RuntimeTypeError>(
				result.getException(),
				std::format(
					"When trying to convert nodeExpression (value=\"{}\") to Storage.",
					NodeFactory::node(rootNodeExpression).value
				),
				"RuntimeTypedExprComponent::fromNodeExpression"
			);

		return RuntimeTypedExprComponent(result.getValue());
	}
	}
	return RuntimeError<RuntimeTypeError>(
		"The conversion from nodeExpression to LambdaFunction failed due to an invalid nodeState.",
		"RuntimeTypedExprComponent::fromNodeExpression");
}

inline std::ostream& operator<<(std::ostream& os, const RuntimeTypedExprComponent& rttexcp) {
	os << rttexcp.toString();
	return os;
}

inline std::vector<std::string_view> splitString(std::string_view in, char sep) {
	std::vector<std::string_view> r;
	r.reserve(std::ranges::count(in, sep) + 1); // optional
	for (auto p = in.begin();; ++p) {
		auto q = p;
		p = std::find(p, in.end(), sep);
		r.emplace_back(q, p);
		if (p == in.end())
			return r;
	}
}

inline bool _fastCheckRuntimeTypeArgumentsType(const RuntimeType& baseType, const std::vector<RuntimeTypedExprComponent>& argumentsCheckType) {
	if (static_cast<uint8_t>(baseType.getBaseType()) > 3)
		throw std::runtime_error("Fatal Error: runtimeEvaluateType cannot be used in an argument and can lead to undefined behaviour. (this will happen only becuase user-written function put in runtimeEvaluateType as a argument somewhere.)");

	if (baseType == RuntimeType::NodePointer)
		return true;

	if (baseType.getBaseType() != RuntimeBaseType::_Storage ||
		baseType.asStorage().size() != argumentsCheckType.size())
		return false;

	
	StorageTypeLookUp argumentsBaseType{ baseType.asStorage() };
	for (size_t ind{ 0 }, len{ argumentsCheckType.size() }; ind < len; ind++) {
		RuntimeBaseType argumentsBaseTypeAtInd{ argumentsBaseType.lookUpAt(ind).begin()->getBaseRuntimeType() };
		if (std::holds_alternative<Number>(argumentsCheckType[ind]) && argumentsBaseTypeAtInd != RuntimeBaseType::Number)
			return false;
		if (std::holds_alternative<Lambda>(argumentsCheckType[ind]) && std::get<Lambda>(argumentsCheckType[ind]).getType() != argumentsBaseTypeAtInd)
			return false;
		if (std::holds_alternative<Storage>(argumentsCheckType[ind]) && std::get<Storage>(argumentsCheckType[ind]).getType() != argumentsBaseTypeAtInd)
			return false;
	}
	return true;
}

inline Result<RuntimeType, std::runtime_error> getExpressionReturnType(NodeFactory::NodePos rootExpressionNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, std::unordered_map<NodeFactory::NodePos, RuntimeType>* nodesTypeCache) {
	if (nodesTypeCache && nodesTypeCache->contains(rootExpressionNode))
		return nodesTypeCache->at(rootExpressionNode);

	std::stack<NodeFactory::NodePos> operationStack;
	std::unordered_map<NodeFactory::NodePos, RuntimeType> resultMap;

	operationStack.push(rootExpressionNode);
	while (!operationStack.empty()) {
		const NodeFactory::NodePos currNodePos{ operationStack.top() };

		if (!NodeFactory::validNode(currNodePos)) {
			operationStack.pop();
			continue;
		}

		const NodeFactory::Node& currNode{ NodeFactory::node(currNodePos) }; // gurantree node to not change value, use reference.

		// if currNode is a leaf node.
		if (!NodeFactory::validNode(currNode.rightPos) &&
			!NodeFactory::validNode(currNode.leftPos)) {
			if (currNode.value == ".")
				resultMap[currNodePos] = RuntimeType::Number;

			else if (EvaluatorLambdaFunctions.contains(currNode.value) &&
				EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Constant) {
				resultMap[currNodePos] = EvaluatorLambdaFunctions.at(currNode.value).getReturnType();
			}

			else if (currNode.nodeState == NodeFactory::Node::NodeState::Storage)
				resultMap[currNodePos] = RuntimeType::HiddenType::_Storage; // null storage

			else
				resultMap[currNodePos] = RuntimeType::Number;
		}

		else if (currNode.nodeState == NodeFactory::Node::NodeState::LambdaFuntion) {
			std::vector<std::pair<std::string, RuntimeType>> parameters{ currNode.parametersWithType };
			std::vector<RuntimeType> returnTypes;
			std::vector<RuntimeType> lambdaParameterType;
			lambdaParameterType.reserve(parameters.size());

			std::unordered_map<std::string, Lambda> tempEvaluatorLambdaFunctions(EvaluatorLambdaFunctions);

			// depend on arguments size in this case the argument size should be either same as parameter or 0.
			for (size_t ind{ 0 }; ind < parameters.size(); ind++) {
				Result<Lambda, std::runtime_error> tempFunc{ Lambda::fromFunction(
					parameters[ind].first,
					RuntimeType::Lambda(
						parameters[ind].second,
						 RuntimeType::HiddenType::_Storage
					),
					Lambda::LambdaNotation::Constant,
					[](const Lambda::LambdaArguments&) {
						return NodeFactory::NodePosNull;
					}
				) };

				if (tempFunc.isError())
					return RuntimeError<LambdaConstructionError>(
						tempFunc.getException(),
						std::format(
							"When attempting to create a constant lambda function \"{}\", which is used to assist in defining the return type",
							parameters[ind].first
						),
						"getReturnType"
					);

				tempEvaluatorLambdaFunctions.try_emplace(parameters[ind].first, tempFunc.moveValue());
				lambdaParameterType.emplace_back(parameters[ind].second);
			}

			NodeFactory::NodePos currArgNodePos{ currNodePos };
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<RuntimeType, std::runtime_error> result{ getExpressionReturnType(NodeFactory::node(currArgNodePos).leftPos, tempEvaluatorLambdaFunctions) };

				// Handle error occur from determining argument type
				if (result.isError())
					return RuntimeError<RuntimeTypeError>(
						result.getException(),
						std::format(
							"While determining argument type of \"{}\"",
							NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
							? NodeFactory::node(currArgNodePos).leftNode().value
							: "Null"
						),
						"getReturnType"
					);

				if (returnTypes.size() &&
					result.getValue().getBaseType() == RuntimeBaseType::_Lambda &&
					returnTypes.back().getBaseType() == RuntimeBaseType::_Storage) {
					returnTypes.pop_back();

					if (returnTypes.back().asLambda().paramsType() != result.getValue())
						return RuntimeError<RuntimeTypeError>(
							std::format(
								"parameters type must be same as argument type! ({} != {})",
								returnTypes.back(),
								result.getValue()),
							"getReturnType"
						);

					returnTypes.emplace_back(returnTypes.back().asLambda().returnType());
				}

				returnTypes.emplace_back(result.getValue());
				currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
			}

			if (lambdaParameterType.empty())
				resultMap[currNodePos] = RuntimeType::Lambda(std::move(returnTypes.back()), RuntimeBaseType::_Storage);
			else if (lambdaParameterType.size() == 1)
				resultMap[currNodePos] = RuntimeType::Lambda(std::move(returnTypes.back()), std::move(lambdaParameterType.front()));
			else
				resultMap[currNodePos] = RuntimeType::Lambda(std::move(returnTypes.back()), RuntimeType::gurantreeNoRuntimeEvaluateStorage(std::move(lambdaParameterType)));
		}

		else if (currNode.nodeState == NodeFactory::Node::NodeState::Storage) {
			std::vector<RuntimeType> arguments;

			NodeFactory::NodePos currArgNodePos{ currNodePos };
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<RuntimeType, std::runtime_error> result{ getExpressionReturnType(NodeFactory::node(currArgNodePos).leftPos, EvaluatorLambdaFunctions) };

				if (result.isError())
					return RuntimeError<RuntimeTypeError>(
						result.getException(),
						std::format("While determining argument type of \"{}\"",
							NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
							? NodeFactory::node(currArgNodePos).leftNode().value
							: "Null"),
						"getReturnType");

				arguments.push_back(result.getValue());
				currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
			}

			resultMap[currNodePos] = RuntimeType::gurantreeNoRuntimeEvaluateStorage(arguments);
		}

		else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Infix) {
			if (!resultMap.contains(currNode.leftPos)) {
				operationStack.push(currNode.leftPos);
				continue;
			}

			if (!resultMap.contains(currNode.rightPos)) {
				operationStack.push(currNode.rightPos);
				continue;
			}

			RuntimeType leftType{ resultMap[currNode.leftPos] };
			RuntimeType rightType{ resultMap[currNode.rightPos] };

			// Lambda parameter numbers is guarantree to be 2 (check at Lambda construction.)
			const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };

			if (lambdaFunction.getParamsType().getBaseType() != RuntimeBaseType::_Stroage_Any) {
				if (const auto parametersType{ lambdaFunction.getParamsType().asStorage() };
					!(parametersType.at(0) == leftType && parametersType.at(1) == rightType))

					return RuntimeError<RuntimeTypeError>(
						std::format("Parameters type must be equal to argument type. ({} != {})",
							lambdaFunction.getParamsType(),
							RuntimeType::gurantreeNoRuntimeEvaluateStorage({ leftType, rightType })),
						"getReturnType");
			}

			resultMap[currNodePos] = lambdaFunction.getReturnType();
		}

		else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Postfix) {
			if (!resultMap.contains(currNode.rightPos)) {
				operationStack.push(currNode.rightPos);
				continue;
			}

			RuntimeType rightVal{ resultMap[currNode.rightPos] };
			const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };

			if (lambdaFunction.getParamsType().getBaseType() != RuntimeBaseType::_Stroage_Any) {
				if (lambdaFunction.getParamsType() != rightVal)
					return RuntimeError<RuntimeTypeError>(
						std::format("Parameters type must be equal to argument type. ({} != {})",
							lambdaFunction.getParamsType(),
							rightVal),
						"getReturnType");
			}

			resultMap[currNodePos] = lambdaFunction.getReturnType();
		}

		else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Prefix) {
			if (!resultMap.contains(currNode.leftPos)) {
				operationStack.push(currNode.leftPos);
				continue;
			}

			RuntimeType leftVal{ resultMap[currNode.leftPos] };
			const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };
			if (lambdaFunction.getParamsType().getBaseType() != RuntimeBaseType::_Stroage_Any) {
				if (lambdaFunction.getParamsType() != leftVal)
					return RuntimeError<RuntimeTypeError>(
						std::format("Parameters type must be equal to argument type. ({} != {})",
							lambdaFunction.getParamsType(),
							leftVal),
						"getReturnType");
			}

			resultMap[currNodePos] = lambdaFunction.getReturnType();
		}

		else {
			// std::cout << currNode.value << std::endl;
		}

		operationStack.pop();
	}

	if (!resultMap.contains(rootExpressionNode))
		return std::runtime_error("failed to find return type.");

	if (nodesTypeCache)
		nodesTypeCache->insert_or_assign(rootExpressionNode, resultMap[rootExpressionNode]);

	return resultMap[rootExpressionNode];
}

#endif //RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY