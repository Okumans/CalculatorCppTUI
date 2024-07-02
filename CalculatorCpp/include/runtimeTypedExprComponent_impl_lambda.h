#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA

#include <sstream>
#include <format>
#include <ranges>
#include <algorithm>
#include <variant>
#include "runtimeTypedExprComponent.h"
#include "runtime_error.h"

inline Lambda::Lambda(const Lambda& other) :
	BaseRuntimeTypedExprComponent(other.getType(), other._getNodeExpression()),
	mLambdaParamsTypeSize{ other.getParamsSize() },
	mLambdaNotation{ other.mLambdaNotation },
	mLambdaFunction{ other.mLambdaFunction },
	mLambdaFunctionSignature{ other.mLambdaFunctionSignature } {}

inline Lambda& Lambda::operator=(const Lambda& other) {
	if (this != &other) {
		mType = other.getType();
		mNodeExpression = other._getNodeExpression();
		mLambdaParamsTypeSize = other.mLambdaParamsTypeSize;
		mLambdaNotation = other.mLambdaNotation;
		mLambdaFunction = other.mLambdaFunction;
		mLambdaFunctionSignature = other.mLambdaFunctionSignature;
	}
	return *this;
}

inline Lambda::Lambda(Lambda&& other) noexcept :
	BaseRuntimeTypedExprComponent(
		std::move(other.mType),
		other.mNodeExpression
	),
	mLambdaParamsTypeSize{ other.mLambdaParamsTypeSize },
	mLambdaNotation{ other.mLambdaNotation },
	mLambdaFunction{ std::move(other.mLambdaFunction) },
	mLambdaFunctionSignature{ std::move(other.mLambdaFunctionSignature) } {}

inline Lambda& Lambda::operator=(Lambda&& other) noexcept {
	if (this != &other) {
		mType = std::move(other.mType);
		mNodeExpression = other._getNodeExpression();
		mLambdaParamsTypeSize = other.mLambdaParamsTypeSize;
		mLambdaNotation = other.mLambdaNotation;
		mLambdaFunction = std::move(other.mLambdaFunction);
		mLambdaFunctionSignature = std::move(other.mLambdaFunctionSignature);
	}
	return *this;
}

inline Lambda::Lambda(
	const std::string& lambdaFunctionSignature,
	const RuntimeType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction) :
	BaseRuntimeTypedExprComponent(lambdaType, NodeFactory::NodePosNull),
	mLambdaParamsTypeSize{ lambdaType.asLambda().paramsSize() },
	mLambdaNotation{ lambdaNotation },
	mLambdaFunction{ std::make_shared<std::function<RuntimeTypedExprComponent(LambdaArguments)>>(lambdaFunction) },
	mLambdaFunctionSignature{ lambdaFunctionSignature }
{
	// gurantree noexcept, assertion will be test in wrapper function
	// assert(mmLambdaInfo.ParamsNumbers == mLambdaParametersName.size());
	// return type will be check at runtime.
}

inline Lambda::Lambda(
	std::string&& lambdaFunctionSignature,
	RuntimeType&& lambdaType,
	LambdaNotation lambdaNotation,
	std::function<RuntimeTypedExprComponent(LambdaArguments)>&& lambdaFunction) :
	BaseRuntimeTypedExprComponent(std::move(lambdaType), NodeFactory::NodePosNull),
	mLambdaParamsTypeSize{ mType.asLambda().paramsSize() },
	mLambdaNotation{ lambdaNotation },
	mLambdaFunction{ std::make_shared<std::function<RuntimeTypedExprComponent(LambdaArguments)>>(std::move(lambdaFunction)) },
	mLambdaFunctionSignature{ std::move(lambdaFunctionSignature) }
{
	// gurantree noexcept, assertion will be test in wrapper function
	// assert(mmLambdaInfo.ParamsNumbers == mLambdaParametersName.size());
	// return type will be check at runtime.
}

inline Lambda::Lambda(
	const RuntimeType& lambdaType,
	LambdaNotation lambdaNotation,
	NodePos lambdaFunctionRootNode) :
	BaseRuntimeTypedExprComponent(lambdaType, lambdaFunctionRootNode),
	mLambdaParamsTypeSize{ lambdaType.asLambda().paramsSize() },
	mLambdaNotation{ lambdaNotation },
	mLambdaFunction{ lambdaFunctionRootNode }
{
	// gurantree noexcept, assertion will be test in wrapper function
	// assert(mmLambdaInfo.ParamsNumbers == mLambdaParametersName.size());
	// assert(NodeFactory::node(lambdaFunctionRootNode).nodestate == NodeFactory::Node::NodeState::LambdaFuntion);
	// return type will be check at runtime.
}

inline Result<Lambda, std::runtime_error> Lambda::fromFunction(
	const std::string& lambdaFunctionSignature,
	const RuntimeType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction)
{
	// Lambda return type will be check at runtime.
	if (lambdaNotation == LambdaNotation::Infix &&
		(lambdaType.getBaseType() != RuntimeBaseType::_Lambda ||
			lambdaType.asLambda().paramsType().getBaseType() != RuntimeBaseType::_Storage ||
			lambdaType.asLambda().paramsSize() != 2))
		return RuntimeError<RuntimeTypeError>(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", lambdaType), "Lambda::fromFunction");
	return Lambda(lambdaFunctionSignature, lambdaType, lambdaNotation, lambdaFunction);
}

inline Result<Lambda, std::runtime_error> Lambda::fromFunction(
	const std::string& lambdaFunctionSignature,
	const RuntimeType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction,
	const LambdaArguments& testArgument
)
{
	if (lambdaNotation == LambdaNotation::Infix &&
		(lambdaType.getBaseType() != RuntimeBaseType::_Lambda ||
			lambdaType.asLambda().paramsType().getBaseType() != RuntimeBaseType::_Storage ||
			lambdaType.asLambda().paramsSize() != 2))
		return RuntimeError<RuntimeTypeError>(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", RuntimeType(lambdaType)), "Lambda::fromFunction");

	const RuntimeType lambdaReturnType{ lambdaType.asLambda().returnType() };

	if (testArgument.size() == lambdaType.asLambda().paramsSize()) {
		try {
			RuntimeTypedExprComponent returnValue = lambdaFunction(testArgument);
			RuntimeType returnType = returnValue.getDetailTypeHold();

			if (returnType == lambdaReturnType)
				return Lambda(lambdaFunctionSignature, lambdaType, lambdaNotation, lambdaFunction);
			return RuntimeError<RuntimeTypeError>(
				std::format("Cannot construct Lambda, Parameter type not matched. ({} != {}))",
					lambdaReturnType,
					returnType
				),
				"Lambda::fromFunction"
			);
		}
		catch (const std::exception& e) {
			return RuntimeError<LambdaEvaluationError>(
				std::format("The lambda evaluation caused an exception \"{}\" during construction.", e.what()),
				"Lambda::fromFunction::mLambdaFunction");
		}
	}

	else
		return RuntimeError<RuntimeTypeError>(
			std::format("Cannot construct Lambda, Parameter amount not matched. ({} != {})",
				lambdaType.asLambda().paramsSize(), testArgument.size()),
			"Lambda::fromFunction"
		);
}

inline Result<Lambda, std::runtime_error> Lambda::fromExpressionNode(
	NodePos lambdaFunctionRootNode,
	const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions)
{
	if (!NodeFactory::validNode(lambdaFunctionRootNode))
		return RuntimeError<RuntimeTypeError>("The conversion from NodeExpression to Lambda failed due to an invalid NodeExpression.",
			"Lambda::fromExpressionNode");

	if (NodeFactory::node(lambdaFunctionRootNode).nodeState == NodeFactory::Node::NodeState::LambdaFuntion) {
		Result<RuntimeType, std::runtime_error>&& returnTypeRaw{ getExpressionReturnType(lambdaFunctionRootNode, EvaluatorLambdaFunctions, &NodeFactory::getNodesCachedType()) };

		if (returnTypeRaw.isError())
			return RuntimeError<RuntimeTypeError>(
				returnTypeRaw.getException(),
				std::format("While determining type of lambdaFunctionRootNode \"{}\"",
					NodeFactory::node(lambdaFunctionRootNode).value),
				"Lambda::fromExpressionNode");

		std::vector<RuntimeType> parameterTypes;
		parameterTypes.reserve(NodeFactory::node(lambdaFunctionRootNode).parametersWithType.size());
		for (const auto& [_, parameterType] : NodeFactory::node(lambdaFunctionRootNode).parametersWithType)
			parameterTypes.emplace_back(parameterType);

		return Lambda(
			returnTypeRaw.getValue(),
			LambdaNotation::Postfix,
			lambdaFunctionRootNode
		);
	}

	else if (NodeFactory::node(lambdaFunctionRootNode).nodeState == NodeFactory::Node::NodeState::Operator) {
		if (EvaluatorLambdaFunctions.contains(NodeFactory::node(lambdaFunctionRootNode).value))
			return EvaluatorLambdaFunctions.at(NodeFactory::node(lambdaFunctionRootNode).value);

		return RuntimeError<LambdaConstructionError>(
			std::format("The LambdaSignature \"{}\" not found in EvaluatorLambdaFunctions",
				NodeFactory::node(lambdaFunctionRootNode).value),
			"Lambda::fromExpressionNode");
	}
	return RuntimeError<LambdaConstructionError>(
		std::format(R"(NodeExpression NodeState should be LambdaFunction("1") or Operator("3") not "{}" (as a number))",
			static_cast<int>(NodeFactory::node(lambdaFunctionRootNode).nodeState)),
		"Lambda::fromExpressionNode");
}

inline Lambda Lambda::LambdaConstant(std::string&& functionSignature, RuntimeTypedExprComponent&& constValue)
{
	return Lambda(
		std::move(functionSignature),
		RuntimeType::Lambda(constValue.getDetailTypeHold(), RuntimeType::HiddenType::_Storage),
		LambdaNotation::Constant,
		[_constValue = std::move(constValue)](const LambdaArguments&) {
			return _constValue;
		}
	);
}

inline Lambda Lambda::LambdaConstant(const std::string& functionSignature, const RuntimeTypedExprComponent& constValue)
{
	return Lambda(
		functionSignature,
		RuntimeType::Lambda(constValue.getDetailTypeHold(), RuntimeType::HiddenType::_Storage),
		LambdaNotation::Constant,
		[constValue](const LambdaArguments&) {
			return constValue;
		}
	);
}

inline Result<NodeFactory::NodePos, std::runtime_error> Lambda::getExpressionTree(const LambdaArguments& arguments) const {
	if (!_fastCheckRuntimeTypeArgumentsType(getType().asLambda().paramsType(), arguments))
		return std::runtime_error(std::format("Lambda parameter and argument not matched. ({} != {})", getType().asLambda().paramsType(), Storage::fromVector(arguments).getType()));

	if (std::holds_alternative<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>>(mLambdaFunction)) {
		const NodePos operatorNode{ NodeFactory::create(mLambdaFunctionSignature.value()) };
		NodeFactory::node(operatorNode).nodeState = NodeFactory::Node::NodeState::Operator;

		if (mLambdaNotation == LambdaNotation::Infix) {
			const NodePos leftArgument{ arguments[0].toNodeExpression() };
			const NodePos rightArgument{ arguments[1].toNodeExpression() };

			NodeFactory::node(operatorNode).leftPos = leftArgument;
			NodeFactory::node(operatorNode).rightPos = rightArgument;
			return operatorNode;
		}

		const NodePos allArgument = Storage::fromVector(arguments).getNodeExpression();
		if (mLambdaNotation == LambdaNotation::Postfix)
			NodeFactory::node(operatorNode).rightPos = allArgument;

		else if (mLambdaNotation == LambdaNotation::Prefix)
			NodeFactory::node(operatorNode).leftPos = allArgument;

		return operatorNode;
	}

	NodeFactory::node(std::get<NodePos>(mLambdaFunction)).rightPos = Storage::fromVector(arguments).getNodeExpression();
	return std::get<NodePos>(mLambdaFunction);
}

inline RuntimeType Lambda::getReturnType() const {
	return getType().asLambda().returnType();
}

inline RuntimeType Lambda::getParamsType() const {
	return getType().asLambda().paramsType();
}

inline size_t Lambda::getParamsSize() const {
	return mLambdaParamsTypeSize;
}

inline std::optional<std::string_view> Lambda::getLambdaSignature() const {
	return mLambdaFunctionSignature;
}

inline Lambda::LambdaNotation Lambda::getNotation() const {
	return mLambdaNotation;
}

inline void Lambda::setNotation(LambdaNotation notation) {
	mLambdaNotation = notation;
}

inline std::string Lambda::toString() const {
	std::ostringstream ss;
	ss << getType();
	return ss.str();
}

inline NodeFactory::NodePos Lambda::generateExpressionTree() const {
	if (!mLambdaFunctionSignature.has_value())
		return std::get<NodePos>(mLambdaFunction);

	const NodePos operatorNode{ NodeFactory::create(mLambdaFunctionSignature.value()) };
	NodeFactory::node(operatorNode).nodeState = NodeFactory::Node::NodeState::Operator;
	return operatorNode;
}

inline void Lambda::findAndReplaceConstant(NodeFactory::NodePos root, const std::unordered_map<std::string, NodeFactory::NodePos>& replacement) {
	std::stack<NodeFactory::NodePos> nodes;
	nodes.push(root);

	while (!nodes.empty()) {
		NodeFactory::NodePos currNode{ nodes.top() }; nodes.pop();
		if (NodeFactory::validNode(NodeFactory::node(currNode).leftPos)) {
			nodes.push(NodeFactory::node(currNode).leftPos);
			if (replacement.contains(NodeFactory::node(currNode).leftNode().value))
				NodeFactory::node(currNode).leftPos = replacement.at(NodeFactory::node(currNode).leftNode().value);
		}

		if (NodeFactory::validNode(NodeFactory::node(currNode).rightPos)) {
			nodes.push(NodeFactory::node(currNode).rightPos);
			if (replacement.contains(NodeFactory::node(currNode).rightNode().value))
				NodeFactory::node(currNode).rightPos = replacement.at(NodeFactory::node(currNode).rightNode().value);
		}
	}
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::_uncheckedEvaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodePos, NodePos>& nodeDependency, const LambdaArguments& arguments) const
{
	if (std::holds_alternative<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>>(mLambdaFunction)) {
		try {
			return (*std::get<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>>(mLambdaFunction))(arguments);
		}
		catch (const std::exception& e) {
			return RuntimeError<LambdaEvaluationError>(
				std::format("The lambda evaluation caused an exception \"{}\" during runtime.", e.what()),
				"Lambda::evaluate");
		}
	}

	// bool returnValueNeedConstantReplacement{ getType().asLambda().returnType().getBaseType() == RuntimeBaseType::_Lambda };

	std::unordered_map<std::string, Lambda> evaluatorLambdaFunctionsSnapshot(EvaluatorLambdaFunctions);
	std::unordered_map<std::string, NodePos> parameterConstantsReplacement;

	const std::vector<std::pair<std::string, RuntimeType>>& parameters{
		NodeFactory::node(std::get<NodePos>(mLambdaFunction)).parametersWithType
	}; // allowed, usage doesn't involve recusive function such as Lambda::evaluate or Lambda::_NodeExpressionEvaluate

	for (size_t ind{ 0 }, len{ parameters.size() }; ind < len; ind++) {
		auto constLambda = [arguments, ind](const Lambda::LambdaArguments&) {
			return arguments[ind];
			};

		Result<Lambda, std::runtime_error> tempFunc{
			Lambda::fromFunction(
				parameters[ind].first,
				RuntimeType::Lambda(
					parameters[ind].second,
					 RuntimeType::HiddenType::_Storage
				),
				Lambda::LambdaNotation::Constant,
				constLambda
			)
		};

		if (tempFunc.isError())
			return RuntimeError<LambdaConstructionError>(
				tempFunc.getException(),
				std::format(
					"When attempting to create a constant lambda function \"{}\", which is used for determine lambda function evaluation result .",
					parameters[ind].first
				),
				"Lambda::evaluate"
			);

		//if (returnValueNeedConstantReplacement) {
		//	std::cout << "replace constant\n";
		//	parameterConstantsReplacement.try_emplace(parameters[ind].first, arguments[ind].toNodeExpression());
		//}

		evaluatorLambdaFunctionsSnapshot.try_emplace(parameters[ind].first, tempFunc.moveValue());
	}

	//if (returnValueNeedConstantReplacement) {
	//	findAndReplaceConstant(std::get<NodePos>(mLambdaFunction), parameterConstantsReplacement);
	//}
	Result<RuntimeTypedExprComponent, std::runtime_error>&& res{
		_NodeExpressionEvaluate(
			std::get<NodePos>(mLambdaFunction),
			evaluatorLambdaFunctionsSnapshot,
			nodeDependency,
			true
		)
	};

	if (res.isError())
		return RuntimeError<LambdaEvaluationError>(
			res.getException(),
			std::format(
				"When attempting to evaluate lambda function on node \"{}\" (value={})",
				NodeFactory::node(std::get<NodePos>(mLambdaFunction)).leftPos,
				NodeFactory::node(std::get<NodePos>(mLambdaFunction)).value
			),
			"Lambda::evaluate"
		);

	RuntimeTypedExprComponent resultValue{ res.getValue() };
	return resultValue;
}

inline void Lambda::_overrideType(RuntimeType&& Ret, RuntimeType&& Params) {
	mType = RuntimeType::Lambda(std::move(Ret), std::move(Params));
}

inline void Lambda::_overrideType(const RuntimeType& Ret, const RuntimeType& Params) {
	mType = RuntimeType::Lambda(Ret, Params);
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodePos, NodePos>& nodeDependency, const LambdaArguments& arguments) const {
	if (arguments.size() != getParamsSize())
		return RuntimeError<RuntimeTypeError>(
			std::format(
				"Lambda parameter and argument amount not matched. ({} != {})",
				getParamsSize(),
				arguments.size()
			),
			"Lambda::evaluate"
		);

	if (arguments.size() == 1) {
		if (getParamsType() != arguments[0].getDetailTypeHold())
			return RuntimeError<RuntimeTypeError>(
				std::format(
					"Lambda parameter and argument type not matched ({} != {}).",
					getParamsType(),
					arguments[0].getDetailTypeHold()
				),
				"Lambda::evaluate"
			);
	}

	else if (getParamsSize() && !_fastCheckRuntimeTypeArgumentsType(getParamsType(), arguments))
		return RuntimeError<RuntimeTypeError>(
			std::format(
				"Lambda parameter and argument type not matched ({} != {}).",
				getParamsType(),
				Storage::fromVector(arguments).getType()
			),
			"Lambda::evaluate"
		);

	return _uncheckedEvaluate(EvaluatorLambdaFunctions, nodeDependency, arguments);
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const LambdaArguments& arguments) const {
	return evaluate(EvaluatorLambdaFunctions, nodeDependencyNull, arguments);
}

template<RuntimeTypedExprComponentRequired ...Args>
inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodePos, NodePos>& nodeDependency, Args&&... arguments) const {
	constexpr size_t count = sizeof...(arguments);
	std::vector<RuntimeTypedExprComponent> tmp;
	tmp.reserve(count);
	(tmp.emplace_back(std::move(std::forward<Args>(arguments))), ...);
	return evaluate(EvaluatorLambdaFunctions, nodeDependency, tmp);
}

template<RuntimeTypedExprComponentRequired ...Args>
inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, Args && ...arguments) const {
	constexpr size_t count = sizeof...(arguments);
	std::vector<RuntimeTypedExprComponent> tmp;
	tmp.reserve(count);
	(tmp.emplace_back(std::move(std::forward<Args>(arguments))), ...);
	return evaluate(EvaluatorLambdaFunctions, nodeDependencyNull, tmp);
}

template<RuntimeTypedExprComponentRequired ...Args>
inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::_uncheckedEvaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodePos, NodePos>& nodeDependency, Args && ...arguments) const {
	constexpr size_t count = sizeof...(arguments);
	std::vector<RuntimeTypedExprComponent> tmp;
	tmp.reserve(count);
	(tmp.emplace_back(std::move(std::forward<Args>(arguments))), ...);
	return _uncheckedEvaluate(EvaluatorLambdaFunctions, nodeDependency, tmp);
}

static bool isLeafNode(NodeFactory::NodePos nodePos) {
	return (!NodeFactory::validNode(NodeFactory::node(nodePos).leftPos) && !NodeFactory::validNode(NodeFactory::node(nodePos).rightPos));
}

inline Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error> Lambda::_NodeExpressionsEvaluator(std::vector<NodePos> rootNodeExpressions, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const std::unordered_map<NodePos, NodePos>& nodeDependency) {
	std::vector<RuntimeTypedExprComponent> evaluationResults;
	std::unordered_map<std::string, Lambda> EvaluatorLambdaFunctionsSnapShot(EvaluatorLambdaFunctions);

	std::ranges::reverse(rootNodeExpressions);

	while (!rootNodeExpressions.empty()) {
		NodePos currNode = rootNodeExpressions.back(); rootNodeExpressions.pop_back();
		NodeFactory::Node currNodeNode = NodeFactory::node(currNode);

		Result<RuntimeTypedExprComponent, std::runtime_error> evaluationResult{
			_NodeExpressionEvaluate(
				currNode,
				EvaluatorLambdaFunctionsSnapShot,
				nodeDependency
			)
		};

		if (evaluationResult.isError())
			return RuntimeError<LambdaEvaluationError>(
				evaluationResult.getException(),
				std::format(
					"When attempting to evaluate the NodeExpression. (nodeExpression value = {})",
					NodeFactory::validNode(currNode) ? NodeFactory::node(currNode).value : "Null"
				),
				"Lambda::_NodeExpressionsEvaluator"
			);

		evaluationResults.emplace_back(evaluationResult.moveValue());
	}

	return evaluationResults;
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::_NodeExpressionEvaluate(
	NodePos rootNodeExpression,
	std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions,
	std::unordered_map<NodePos, NodePos> nodeDependency,
	bool forceWithArgument
) {
	std::stack<NodeFactory::NodePos> operationStack;
	std::unordered_map<NodeFactory::NodePos, std::optional<RuntimeTypedExprComponent>> resultMap;

	std::unordered_map<NodePos, NodePos> reversedNodeDependency;
	for (const auto& [key, value] : nodeDependency)
		reversedNodeDependency.try_emplace(value, key);

	operationStack.push(rootNodeExpression);
	while (!operationStack.empty()) {
		const NodeFactory::NodePos currNodePos{ operationStack.top() };

		if (!NodeFactory::validNode(currNodePos)) {
			operationStack.pop();
			continue;
		}

		NodeFactory::Node* currNode{ &NodeFactory::node(currNodePos) };

		if (nodeDependency.contains(currNodePos)) {
			operationStack.emplace(nodeDependency.at(currNodePos));
			continue;
		}

		// if currNode is a leaf node.
		if (!NodeFactory::validNode(currNode->rightPos) &&
			!NodeFactory::validNode(currNode->leftPos)) {
			if (currNode->value == "." || currNode->value == "-.")
				resultMap[currNodePos] = 0;

			else if (currNode->nodeState == NodeFactory::Node::NodeState::Storage)
				resultMap[currNodePos] = Storage::NullStorage();

			else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
				EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Constant)
			{
				Lambda constOperator{ EvaluatorLambdaFunctions.at(currNode->value) };
				Result<RuntimeTypedExprComponent, std::runtime_error>&& constOperatorResult{ constOperator.evaluate(EvaluatorLambdaFunctions, {}) };

				currNode = &NodeFactory::node(currNodePos); // update currNode, evaluate can change currNode address.

				if (constOperatorResult.isError())
					return RuntimeError<LambdaEvaluationError>(
						constOperatorResult.getException(),
						std::format(
							"When attempting to evaluate the constant lambda function with leaf node constant operator evaluation. (constant operator nodeExpression value = {})",
							currNode->value
						),
						"Lambda::_NodeExpressionEvaluate"
					);

				resultMap[currNodePos] = constOperatorResult.moveValue();
			}
			else
				resultMap[currNodePos] = std::stold(currNode->value);
		}

		else if (currNode->nodeState == NodeFactory::Node::NodeState::LambdaFuntion) {
			std::vector<NodeFactory::NodePos> expressions;

			if (currNode->parametersWithType.size() && !forceWithArgument) {
				Result<Lambda, std::runtime_error> lambdaFunctionResult{
					Lambda::fromExpressionNode(currNodePos, EvaluatorLambdaFunctions)
				};

				if (lambdaFunctionResult.isError())
					return RuntimeError<LambdaEvaluationError>(
						lambdaFunctionResult.getException(),
						std::format(
							"When attempting to convert a NodeExpression into a lambda function for evaluation. (nodeExpression value = {})",
							currNode->value
						),
						"Lambda::_NodeExpressionsEvaluator"
					);

				resultMap[currNodePos] = lambdaFunctionResult.moveValue();
			}

			else {
				NodeFactory::NodePos currArgNodePos{ currNodePos };
				while (NodeFactory::validNode(currArgNodePos)) {
					expressions.push_back(NodeFactory::node(currArgNodePos).leftPos);
					currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
				}

				Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error>&& leftVal{
					_NodeExpressionsEvaluator(expressions, EvaluatorLambdaFunctions, nodeDependency)
				};

				if (leftVal.isError())
					return RuntimeError<LambdaEvaluationError>(
						leftVal.getException(),
						std::format(
							"When attempting to evaluate expession content of a lambda function. (value = {})",
							(NodeFactory::validNode(NodeFactory::node(currNodePos).leftPos)
								? NodeFactory::node(currNodePos).leftNode().value
								: "Null")
						),
						"Lambda::_NodeExpressionEvaluate"
					);

				resultMap[currNodePos] = leftVal.getValue().back();
			}
		}

		else if (currNode->nodeState == NodeFactory::Node::NodeState::Storage) {
			std::vector<RuntimeTypedExprComponent> arguments;

			NodeFactory::NodePos currArgNodePos{ currNodePos };
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<RuntimeTypedExprComponent, std::runtime_error>&& result{
					_NodeExpressionEvaluate(
						NodeFactory::node(currArgNodePos).leftPos,
						EvaluatorLambdaFunctions,
						nodeDependency
					)
				};

				if (result.isError())
					return RuntimeError<StorageEvaluationError>(
						result.getException(),
						std::format(
							"When attempting to evaluate an argument. (value = {})",
							NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
							? NodeFactory::node(currArgNodePos).leftNode().value
							: "Null"),
						"Lambda::_NodeExpressionEvaluate"
					);

				arguments.emplace_back(result.moveValue());
				currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
			}

			if (!arguments.size())
				return std::runtime_error("Cannot evalutate noting.");

			resultMap[currNodePos] = Storage::fromVector(std::move(arguments));
		}

		else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
			EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Infix) {
			if (!resultMap.contains(currNode->leftPos)) {
				operationStack.push(currNode->leftPos);
				continue;
			}

			if (!resultMap.contains(currNode->rightPos)) {
				operationStack.push(currNode->rightPos);
				continue;
			}

			RuntimeTypedExprComponent&& leftVal{ std::move(resultMap[currNode->leftPos].value()) };
			RuntimeTypedExprComponent&& rightVal{ std::move(resultMap[currNode->rightPos].value()) };

			const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode->value) };

			if (lambdaFunction.getParamsType().getBaseType() != RuntimeBaseType::_Stroage_Any) {
				if (lambdaFunction.getParamsSize() != 1)
				{
					StorageTypeLookUp parametersType{ lambdaFunction.getParamsType().asStorage() };
					RuntimeType firstParameter{ parametersType.at(0) };
					RuntimeType secondParameter{ parametersType.at(1) };

					// implicit convert to nodePointer
					if (firstParameter == RuntimeType::NodePointer)
						leftVal = NodePointer(leftVal.toNodeExpression());

					// implicit convert to nodePointer
					if (secondParameter == RuntimeType::NodePointer)
						rightVal = NodePointer(rightVal.toNodeExpression());

					if (!(firstParameter == leftVal.getDetailTypeHold() &&
						secondParameter == rightVal.getDetailTypeHold()))
						return RuntimeError<RuntimeTypeError>(
							std::format(
								"Parameters type must be same as to argument type. ({} != {})",
								lambdaFunction.getParamsType(),
								RuntimeType::gurantreeNoRuntimeEvaluateStorage({
									leftVal.getDetailTypeHold(),
									rightVal.getDetailTypeHold()
									})

							),
							"Lambda::_NodeExpressionEvaluate"
						);
				}
			}

			Result<RuntimeTypedExprComponent, std::runtime_error>&& infixOperatorResult{
				lambdaFunction._uncheckedEvaluate(EvaluatorLambdaFunctions, nodeDependency, std::move(leftVal), std::move(rightVal))
			};

			if (infixOperatorResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					infixOperatorResult.getException(),
					std::format(
						"When attempting to evaluate the infix lambda function. (left nodeExpression value = {}, right nodeExpression value = {})",
						resultMap[currNode->leftPos].value(),
						resultMap[currNode->rightPos].value()
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			resultMap[currNodePos] = infixOperatorResult.moveValue();
		}

		else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
			EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Postfix) {
			if (!resultMap.contains(currNode->rightPos)) {
				operationStack.push(currNode->rightPos);
				continue;
			}

			RuntimeTypedExprComponent&& rightVal{ std::move(resultMap[currNode->rightPos].value()) };
			const Lambda& lambdaFunction = EvaluatorLambdaFunctions.at(currNode->value);

			if (RuntimeType paramsType{ lambdaFunction.getParamsType() };
				paramsType != RuntimeType::HiddenType::_Stroage_Any) {
				// implicit convert to nodePointer
				if (paramsType == RuntimeBaseType::NodePointer)
					rightVal = NodePointer(rightVal.toNodeExpression());

				if (paramsType != rightVal.getDetailTypeHold())
					return RuntimeError<RuntimeTypeError>(
						std::format(
							"Parameters type must be equal to argument type. ({} != {})",
							paramsType,
							rightVal.getDetailTypeHold()
						),
						"Lambda::_NodeExpressionEvaluate"
					);
			}

			Result<RuntimeTypedExprComponent, std::runtime_error>&& postfixOperatorResult{
				lambdaFunction._uncheckedEvaluate(EvaluatorLambdaFunctions, nodeDependency, std::move(rightVal))
			};

			if (postfixOperatorResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					postfixOperatorResult.getException(),
					std::format(
						"When attempting to evaluate the postfix lambda function. (right nodeExpression value = {})",
						resultMap[currNode->rightPos].value()
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			resultMap[currNodePos] = postfixOperatorResult.moveValue();
		}

		else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
			EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Prefix) {
			if (!resultMap.contains(currNode->leftPos)) {
				operationStack.push(currNode->leftPos);
				continue;
			}

			RuntimeTypedExprComponent&& leftVal{ std::move(resultMap[currNode->leftPos].value()) };
			const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode->value) };

			// implicit convert to nodePointer
			if (lambdaFunction.getParamsType() == RuntimeType::NodePointer)
				leftVal = NodePointer(leftVal.toNodeExpression());

			if (lambdaFunction.getParamsType() != leftVal.getDetailTypeHold())
				return RuntimeError<RuntimeTypeError>(
					std::format(
						"Parameters type must be equal to argument type. ({} != {})",
						lambdaFunction.getParamsType(),
						leftVal.getDetailTypeHold()
					)
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& prefixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, nodeDependency, std::move(leftVal))
			};

			if (prefixOperatorResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					prefixOperatorResult.getException(),
					std::format(
						"When attempting to evaluate the prefix lambda function. (left nodeExpression value = {})",
						resultMap[currNode->leftPos].value()
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			resultMap[currNodePos] = prefixOperatorResult.moveValue();
		}

		if (reversedNodeDependency.contains(currNodePos)) {
			nodeDependency.erase(reversedNodeDependency.at(currNodePos));
			EvaluatorLambdaFunctions.insert_or_assign(NodeFactory::node(reversedNodeDependency.at(currNodePos)).value, resultMap[currNodePos].value().getLambda());
		}
		operationStack.pop();
	}

	if (!resultMap.contains(rootNodeExpression))
		return std::runtime_error("failed.");

	return resultMap[rootNodeExpression].value();
}

#endif //RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA