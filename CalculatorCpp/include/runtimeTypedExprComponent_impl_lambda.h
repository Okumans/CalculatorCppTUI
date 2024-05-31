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
	mLambdaInfo{ other.mLambdaInfo },
	mLambdaNotation{ other.mLambdaNotation },
	mLambdaFunction{ other.mLambdaFunction },
	mLambdaFunctionSignature{ other.mLambdaFunctionSignature } {}

inline Lambda& Lambda::operator=(const Lambda& other) {
	if (this != &other) {
		mType = other.getType();
		mNodeExpression = other._getNodeExpression();
		mLambdaNotation = other.mLambdaNotation;
		mLambdaInfo = other.mLambdaInfo;
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
	mLambdaInfo(std::move(other.mLambdaInfo)),
	mLambdaNotation(other.mLambdaNotation),
	mLambdaFunction(std::move(other.mLambdaFunction)),
	mLambdaFunctionSignature(std::move(other.mLambdaFunctionSignature)) {}

inline Lambda& Lambda::operator=(Lambda&& other) noexcept {
	if (this != &other) {
		mType = std::move(other.mType);
		mNodeExpression = other._getNodeExpression();
		mLambdaInfo = std::move(other.mLambdaInfo);
		mLambdaNotation = other.mLambdaNotation;
		mLambdaFunction = std::move(other.mLambdaFunction);
		mLambdaFunctionSignature = std::move(other.mLambdaFunctionSignature);
	}
	return *this;
}

inline Lambda::Lambda(
	const std::string& lambdaFunctionSignature,
	const RuntimeCompoundType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction) :
	BaseRuntimeTypedExprComponent(lambdaType, NodeFactory::NodePosNull),
	mLambdaInfo{ RuntimeCompoundType::getLambdaInfo(lambdaType) },
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
	RuntimeCompoundType&& lambdaType,
	LambdaNotation lambdaNotation,
	std::function<RuntimeTypedExprComponent(LambdaArguments)>&& lambdaFunction) :
	BaseRuntimeTypedExprComponent(std::move(lambdaType), NodeFactory::NodePosNull),
	mLambdaInfo{ RuntimeCompoundType::getLambdaInfo(mType) },
	mLambdaNotation{ lambdaNotation },
	mLambdaFunction{ std::make_shared<std::function<RuntimeTypedExprComponent(LambdaArguments)>>(std::move(lambdaFunction)) },
	mLambdaFunctionSignature{ std::move(lambdaFunctionSignature) }
{
	// gurantree noexcept, assertion will be test in wrapper function
	// assert(mmLambdaInfo.ParamsNumbers == mLambdaParametersName.size());
	// return type will be check at runtime.
}

inline Lambda::Lambda(
	const RuntimeCompoundType& lambdaType,
	LambdaNotation lambdaNotation,
	NodePos lambdaFunctionRootNode) :
	BaseRuntimeTypedExprComponent(lambdaType, lambdaFunctionRootNode),
	mLambdaInfo{ RuntimeCompoundType::getLambdaInfo(lambdaType) },
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
	const RuntimeCompoundType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction)
{
	// Lambda return type will be check at runtime.
	if (lambdaNotation == LambdaNotation::Infix &&
		(lambdaType.Type != RuntimeBaseType::_Lambda ||
			std::get_if<RuntimeCompoundType>(&lambdaType.Children[1])->Type != RuntimeBaseType::_Storage ||
			RuntimeCompoundType::_getLambdaParamsNumbers(lambdaType) != 2))
		return RuntimeError<RuntimeTypeError>(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", RuntimeType(lambdaType)), "Lambda::fromFunction");
	return Lambda(lambdaFunctionSignature, lambdaType, lambdaNotation, lambdaFunction);
}

inline Result<Lambda, std::runtime_error> Lambda::fromFunction(
	const std::string& lambdaFunctionSignature,
	const RuntimeCompoundType& lambdaType,
	LambdaNotation lambdaNotation,
	const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction,
	const LambdaArguments& testArgument
)
{
	if (lambdaNotation == LambdaNotation::Infix &&
		(lambdaType.Type != RuntimeBaseType::_Lambda ||
			std::get_if<RuntimeCompoundType>(&lambdaType.Children[1])->Type != RuntimeBaseType::_Storage ||
			RuntimeCompoundType::_getLambdaParamsNumbers(lambdaType) != 2))
		return RuntimeError<RuntimeTypeError>(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", RuntimeType(lambdaType)), "Lambda::fromFunction");

	const RuntimeType lambdaReturnType{ RuntimeCompoundType::_getLambdaReturnType(lambdaType) };
	size_t lambdaParamsNumbers{ RuntimeCompoundType::_getLambdaParamsNumbers(lambdaType) };

	if (testArgument.size() == lambdaParamsNumbers) {
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
				lambdaParamsNumbers, testArgument.size()),
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

	if (NodeFactory::node(lambdaFunctionRootNode).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
		Result<RuntimeType, std::runtime_error>&& returnTypeRaw{ getReturnType(lambdaFunctionRootNode, EvaluatorLambdaFunctions) };

		if (returnTypeRaw.isError())
			return RuntimeError<RuntimeTypeError>(
				returnTypeRaw.getException(),
				std::format("While determining type of lambdaFunctionRootNode \"{}\"",
					NodeFactory::node(lambdaFunctionRootNode).value),
				"Lambda::fromExpressionNode");

		std::vector<RuntimeType> parameterTypes;
		parameterTypes.reserve(NodeFactory::node(lambdaFunctionRootNode).utilityStorage.size());
		for (const auto& [_, parameterType] : NodeFactory::node(lambdaFunctionRootNode).utilityStorage)
			parameterTypes.emplace_back(parameterType);

		return Lambda(
			std::get<RuntimeCompoundType>(returnTypeRaw.getValue()),
			LambdaNotation::Postfix,
			lambdaFunctionRootNode
		);
	}

	else if (NodeFactory::node(lambdaFunctionRootNode).nodestate == NodeFactory::Node::NodeState::Operator) {
		if (EvaluatorLambdaFunctions.contains(NodeFactory::node(lambdaFunctionRootNode).value))
			return EvaluatorLambdaFunctions.at(NodeFactory::node(lambdaFunctionRootNode).value);

		return RuntimeError<LambdaConstructionError>(
			std::format("The LambdaSignature \"{}\" not found in EvaluatorLambdaFunctions",
				NodeFactory::node(lambdaFunctionRootNode).value),
			"Lambda::fromExpressionNode");
	}
	return RuntimeError<LambdaConstructionError>(
		std::format(R"(NodeExpression NodeState should be LambdaFunction("1") or Operator("3") not "{}" (as a number))",
			static_cast<int>(NodeFactory::node(lambdaFunctionRootNode).nodestate)),
		"Lambda::fromExpressionNode");
}

inline Lambda Lambda::LambdaConstant(std::string&& functionSignature, RuntimeTypedExprComponent&& constValue)
{
	return Lambda(
		std::move(functionSignature),
		RuntimeCompoundType::Lambda(constValue.getDetailTypeHold(), RuntimeBaseType::_Storage),
		LambdaNotation::Constant,
		[constValue = std::move(constValue)](const LambdaArguments&) -> RuntimeTypedExprComponent {
			return constValue;
		}
	);
}

inline Lambda Lambda::LambdaConstant(const std::string& functionSignature, const RuntimeTypedExprComponent& constValue)
{
	return Lambda(
		functionSignature,
		RuntimeCompoundType::Lambda(constValue.getDetailTypeHold(), RuntimeBaseType::_Storage),
		LambdaNotation::Constant,
		[constValue](const LambdaArguments&) -> RuntimeTypedExprComponent {
			return constValue;
		}
	);
}

inline Result<NodeFactory::NodePos, std::runtime_error> Lambda::getExpressionTree(const LambdaArguments& arguments) const {
	if (!_fastCheckRuntimeTypeArgumentsType(*mLambdaInfo.ParamsType, arguments))
		return std::runtime_error(std::format("Lambda parameter and argument not matched. ({} != {})", *mLambdaInfo.ParamsType, Storage::fromVector(arguments).getType()));

	if (std::holds_alternative<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>>(mLambdaFunction)) {
		const NodePos operatorNode{ NodeFactory::create(mLambdaFunctionSignature.value()) };
		NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;

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

inline RuntimeCompoundType::LambdaInfo Lambda::getLambdaInfo() const {
	return mLambdaInfo;
}

inline std::optional<std::string_view> Lambda::getLambdaSignature() const {
	return mLambdaFunctionSignature;
}

inline Lambda::LambdaNotation Lambda::getNotation() const {
	return mLambdaNotation;
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
	NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;
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

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const LambdaArguments& arguments) const {
	if (arguments.size() != mLambdaInfo.ParamsNumbers)
		return RuntimeError<RuntimeTypeError>(
			std::format(
				"Lambda parameter and argument amount not matched. ({} != {})",
				mLambdaInfo.ParamsNumbers,
				arguments.size()
			),
			"Lambda::evaluate"
		);

	if (arguments.size() == 1) {
		if (*mLambdaInfo.ParamsType != arguments[0].getDetailTypeHold())
			return RuntimeError<RuntimeTypeError>(
				std::format(
					"Lambda parameter and argument type not matched ({} != {}).",
					*mLambdaInfo.ParamsType,
					arguments[0].getDetailTypeHold()
				),
				"Lambda::evaluate"
			);
	}

	else if (mLambdaInfo.ParamsNumbers && !_fastCheckRuntimeTypeArgumentsType(*mLambdaInfo.ParamsType, arguments))
		return RuntimeError<RuntimeTypeError>(
			std::format(
				"Lambda parameter and argument type not matched ({} != {}).",
				*mLambdaInfo.ParamsType,
				Storage::fromVector(arguments).getType()
			),
			"Lambda::evaluate"
		);

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

	bool returnValueNeedConstantReplacement = (std::holds_alternative<RuntimeCompoundType>(*mLambdaInfo.ReturnType) && std::get<RuntimeCompoundType>(*mLambdaInfo.ReturnType).Type == RuntimeBaseType::_Lambda);

	std::unordered_map<std::string, Lambda> evaluatorLambdaFunctionsSnapshot(EvaluatorLambdaFunctions);
	std::unordered_map<std::string, NodePos> parameterConstantsReplacement;

	const std::vector<std::pair<std::string, RuntimeType>>& parameters{
		NodeFactory::node(std::get<NodePos>(mLambdaFunction)).utilityStorage
	}; // allowed, usage doesn't involve recusive function such as Lambda::evaluate or Lambda::_NodeExpressionEvaluate

	for (size_t ind{ 0 }, len{ parameters.size() }; ind < len; ind++) {
		auto constLambda = [arguments, ind](const Lambda::LambdaArguments&) {
			return arguments[ind];
			};

		Result<Lambda, std::runtime_error> tempFunc{
			Lambda::fromFunction(
				parameters[ind].first,
				RuntimeCompoundType::Lambda(
					parameters[ind].second,
					RuntimeBaseType::_Storage
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

		if (returnValueNeedConstantReplacement)
			parameterConstantsReplacement.try_emplace(parameters[ind].first, arguments[ind].toNodeExpression());

		evaluatorLambdaFunctionsSnapshot.try_emplace(parameters[ind].first, tempFunc.moveValue());
	}

	if (returnValueNeedConstantReplacement)
		findAndReplaceConstant(std::get<NodePos>(mLambdaFunction), parameterConstantsReplacement);

	Result<RuntimeTypedExprComponent, std::runtime_error>&& res{
		_NodeExpressionEvaluate(
			std::get<NodePos>(mLambdaFunction),
			evaluatorLambdaFunctionsSnapshot
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

template<RuntimeTypedExprComponentRequired ...Args>
inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, Args&&... arguments) const {
	constexpr size_t count = sizeof...(arguments);
	std::vector<RuntimeTypedExprComponent> tmp;
	tmp.reserve(count);
	(tmp.emplace_back(std::move(std::forward<Args>(arguments))), ...);
	return evaluate(EvaluatorLambdaFunctions, tmp);
}

static bool isLeafNode(NodeFactory::NodePos nodePos) {
	return (!NodeFactory::validNode(NodeFactory::node(nodePos).leftPos) && !NodeFactory::validNode(NodeFactory::node(nodePos).rightPos));
}

inline Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error> Lambda::_NodeExpressionsEvaluator(std::vector<NodePos> rootNodeExpressions, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions) {
	std::vector<RuntimeTypedExprComponent> evaluationResults;

	std::ranges::reverse(rootNodeExpressions);

	while (!rootNodeExpressions.empty()) {
		NodePos currNode = rootNodeExpressions.back(); rootNodeExpressions.pop_back();
		NodeFactory::Node currNodeNode = NodeFactory::node(currNode);
		if (NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::LambdaFuntion ||
			(NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::Operator && isLeafNode(currNode))) {
			Result<Lambda, std::runtime_error> lambdaFunctionResult{ Lambda::fromExpressionNode(currNode, EvaluatorLambdaFunctions) };
			if (lambdaFunctionResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					lambdaFunctionResult.getException(),
					std::format(
						"When attempting to convert a NodeExpression into a lambda function for evaluation. (nodeExpression value = {})",
						NodeFactory::validNode(currNode) ? NodeFactory::node(currNode).value : "Null"
					),
					"Lambda::_NodeExpressionsEvaluator"
				);

			// is a operator so, argument doesnt need to be a storage
			if (NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::Operator &&
				isLeafNode(currNode) &&
				lambdaFunctionResult.getValue().getNotation() == Lambda::LambdaNotation::Prefix &&
				evaluationResults.size()) {
				Result<RuntimeTypedExprComponent, std::runtime_error> evaluationEvaluationResult{
					lambdaFunctionResult.getValue().evaluate(EvaluatorLambdaFunctions, evaluationResults.back())
				};

				if (evaluationEvaluationResult.isError())
					return RuntimeError<LambdaEvaluationError>(
						evaluationEvaluationResult.getException(),
						std::format(
							R"(When evaluating the prefix operator lambda function "{}" with the argument "{}".)",
							lambdaFunctionResult.getValue().toString(),
							evaluationResults.back().toString()
						),
						"Lambda::_NodeExpressionsEvaluator"
					);

				evaluationResults.pop_back();
				evaluationResults.emplace_back(evaluationEvaluationResult.moveValue());
				continue;
			}

			evaluationResults.emplace_back(lambdaFunctionResult.moveValue());
			continue;
		}

		if (NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::Storage) {
			Result<Storage, std::runtime_error> storageResult{
				Storage::fromExpressionNode(currNode, EvaluatorLambdaFunctions) };

			if (storageResult.isError())
				return RuntimeError<StorageEvaluationError>(
					storageResult.getException(),
					std::format(
						"When attempting to convert a NodeExpression into a storage for evaluation. (nodeExpression value = {})",
						NodeFactory::validNode(currNode) ? NodeFactory::node(currNode).value : "Null"
					),
					"Lambda::_NodeExpressionsEvaluator"
				);

			if (evaluationResults.size() && evaluationResults.back().getTypeHolded() == RuntimeBaseType::_Lambda &&
				evaluationResults.back().getLambda().getNotation() == LambdaNotation::Postfix) {
				Lambda lambdaFunction{ evaluationResults.back().getLambda() };
				Result<RuntimeTypedExprComponent, std::runtime_error> evalutationResult{
					lambdaFunction.evaluate(EvaluatorLambdaFunctions, storageResult.getValue().getData())
				};

				if (evalutationResult.isError())
					return RuntimeError<LambdaEvaluationError>(
						evalutationResult.getException(),
						std::format(
							R"(When evaluating the lambda function "{}" with the argument "{}".)",
							evaluationResults.back().toString(),
							storageResult.getValue().toString()
						),
						"Lambda::_NodeExpressionsEvaluator"
					);

				std::cout << evalutationResult.getValue() << "";

				evaluationResults.pop_back();
				evaluationResults.emplace_back(evalutationResult.moveValue());
				continue;
			}

			if (evaluationResults.size() && evaluationResults.back().getTypeHolded() == RuntimeBaseType::_Lambda &&
				evaluationResults.back().getLambda().getNotation() == LambdaNotation::Infix) {
				if (evaluationResults.size() < 2)
					return RuntimeError<LambdaEvaluationError>(
						R"(When evaluating the infix operator lambda function "{}", but argument size is not correct (2 != 1))",
						"Lambda::_NodeExpressionsEvaluator"
					);

				Lambda lambdaFunction{ evaluationResults.back().getLambda() };
				Result<RuntimeTypedExprComponent, std::runtime_error> evalutationResult{
					lambdaFunction.evaluate(
						EvaluatorLambdaFunctions,
						std::move(evaluationResults[evaluationResults.size() - 2]),
						storageResult.moveValue())
				};

				if (evalutationResult.isError())
					return RuntimeError<LambdaEvaluationError>(
						evalutationResult.getException(),
						R"(When evaluating the lambda function "<moved value>" with the argument "<moved value>".)",
						"Lambda::_NodeExpressionsEvaluator"
					);

				evaluationResults.pop_back();
				evaluationResults.pop_back();
				evaluationResults.emplace_back(evalutationResult.moveValue());
				continue;
			}

			evaluationResults.emplace_back(storageResult.moveValue());
			continue;
		};

		Result<RuntimeTypedExprComponent, std::runtime_error> evaluationResult{
			_NodeExpressionEvaluate(
				currNode,
				EvaluatorLambdaFunctions
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

		if (evaluationResult.getValue().getTypeHolded() == RuntimeBaseType::_Storage &&
			evaluationResults.size() && evaluationResults.back().getTypeHolded() == RuntimeBaseType::_Lambda &&
			evaluationResults.back().getLambda().getNotation() == LambdaNotation::Postfix &&
			NodeFactory::node(evaluationResults.back().getLambda().getNodeExpression()).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
			Result<RuntimeTypedExprComponent, std::runtime_error> evaluationEvaluationResult{
				evaluationResults.back().getLambda().evaluate(EvaluatorLambdaFunctions, evaluationResult.getValue().getStorage().getData())
			};

			if (evaluationEvaluationResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					evaluationEvaluationResult.getException(),
					std::format(
						R"(When evaluating the lambda function "{}" with the argument "{}".)",
						evaluationResults.back().toString(),
						evaluationResult.getValue().toString()
					),
					"Lambda::_NodeExpressionsEvaluator"
				);

			evaluationResults.pop_back();
			evaluationResults.emplace_back(evaluationEvaluationResult.moveValue());
			continue;
		}

		if (evaluationResults.size() && evaluationResults.back().getTypeHolded() == RuntimeBaseType::_Lambda &&
			evaluationResults.back().getLambda().getNotation() == LambdaNotation::Postfix &&
			NodeFactory::node(evaluationResults.back().getLambda().getNodeExpression()).nodestate == NodeFactory::Node::NodeState::Operator) {
			Result<RuntimeTypedExprComponent, std::runtime_error> evaluationEvaluationResult{
				evaluationResults.back().getLambda().evaluate(EvaluatorLambdaFunctions, evaluationResult.moveValue())
			};

			if (evaluationEvaluationResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					evaluationEvaluationResult.getException(),
					std::format(
						R"(When evaluating the postfix operator lambda function "{}" with the argument "<move value>".)",
						evaluationResults.back().toString()
					),
					"Lambda::_NodeExpressionsEvaluator"
				);

			evaluationResults.pop_back();
			evaluationResults.emplace_back(evaluationEvaluationResult.moveValue());
			continue;
		}

		// is a operator (not runtime user-create lambda function)
		if (evaluationResults.size() && evaluationResults.back().getTypeHolded() == RuntimeBaseType::_Lambda &&
			evaluationResults.back().getLambda().getNotation() == LambdaNotation::Infix &&
			NodeFactory::node(evaluationResults.back().getLambda().getNodeExpression()).nodestate == NodeFactory::Node::NodeState::Operator) {
			if (evaluationResults.size() < 2)
				return RuntimeError<LambdaEvaluationError>(
					R"(When evaluating the infix operator lambda function "{}", but argument size is not correct (2 != 1))",
					"Lambda::_NodeExpressionsEvaluator"
				);

			Result<RuntimeTypedExprComponent, std::runtime_error> evalutationEvaluationResult{
				evaluationResults.back().getLambda().evaluate(
					EvaluatorLambdaFunctions,
					std::move(evaluationResults[evaluationResults.size() - 2]),
					evaluationResult.moveValue())
			};

			if (evalutationEvaluationResult.isError())
				return RuntimeError<LambdaEvaluationError>(
					evalutationEvaluationResult.getException(),
					R"(When evaluating the infix operator lambda function "<moved value>" with the argument "<moved value>".)",
					"Lambda::_NodeExpressionsEvaluator"
				);

			evaluationResults.pop_back();
			evaluationResults.pop_back();
			evaluationResults.emplace_back(evalutationEvaluationResult.moveValue());
			continue;
		}

		evaluationResults.emplace_back(evaluationResult.moveValue());
	}

	return evaluationResults;
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::_NodeExpressionEvaluate(
	NodePos rootNodeExpression,
	const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions
) {
	std::stack<NodeFactory::NodePos> operationStack;
	std::unordered_map<NodeFactory::NodePos, std::optional<RuntimeTypedExprComponent>> resultMap;

	operationStack.push(rootNodeExpression);
	while (!operationStack.empty()) {
		const NodeFactory::NodePos currNodePos{ operationStack.top() };

		if (!NodeFactory::validNode(currNodePos)) {
			operationStack.pop();
			continue;
		}

		NodeFactory::Node* currNode{ &NodeFactory::node(currNodePos) };

		// if currNode is a leaf node.
		if (!NodeFactory::validNode(currNode->rightPos) &&
			!NodeFactory::validNode(currNode->leftPos)) {
			if (currNode->value == "." || currNode->value == "-.")
				resultMap[currNodePos] = 0;

			else if (currNode->nodestate == NodeFactory::Node::NodeState::Storage)
				resultMap[currNodePos] = Storage::NullStorage();

			else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
				EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Constant) {
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

		else if (currNode->nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
			std::vector<NodeFactory::NodePos> expressions;

			bool evalutateState = true;
			for (const auto& [parameterName, _] : currNode->utilityStorage)
				evalutateState = evalutateState && EvaluatorLambdaFunctions.contains(parameterName);

			if (!evalutateState) {
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
					_NodeExpressionsEvaluator(expressions, EvaluatorLambdaFunctions)
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

		else if (currNode->nodestate == NodeFactory::Node::NodeState::Storage) {
			std::vector<RuntimeTypedExprComponent> arguments;

			NodeFactory::NodePos currArgNodePos{ currNodePos };
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<RuntimeTypedExprComponent, std::runtime_error>&& result{
					_NodeExpressionEvaluate(
						NodeFactory::node(currArgNodePos).leftPos,
						EvaluatorLambdaFunctions
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
			const auto& parametersType{
				RuntimeCompoundType::getStorageInfo(
					*lambdaFunction.getLambdaInfo().ParamsType
				).Storage
			};

			// implicit convert to nodePointer
			if ((*parametersType)[0] == RuntimeBaseType::NodePointer)
				leftVal = NodePointer(leftVal.toNodeExpression());

			// implicit convert to nodePointer
			if ((*parametersType)[1] == RuntimeBaseType::NodePointer)
				rightVal = NodePointer(rightVal.toNodeExpression());

			if (!((*parametersType)[0] == leftVal.getDetailTypeHold() &&
				(*parametersType)[1] == rightVal.getDetailTypeHold()))
				return RuntimeError<RuntimeTypeError>(
					std::format(
						"Parameters type must be same as to argument type. ({} != {})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						RuntimeType(
							RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage({
								leftVal.getDetailTypeHold(),
								rightVal.getDetailTypeHold()
								})
						)
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& infixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, std::move(leftVal), std::move(rightVal))
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

			// implicit convert to nodePointer
			if (*lambdaFunction.getLambdaInfo().ParamsType == RuntimeBaseType::NodePointer)
				rightVal = NodePointer(rightVal.toNodeExpression());

			if (*lambdaFunction.getLambdaInfo().ParamsType != rightVal.getDetailTypeHold())
				return RuntimeError<RuntimeTypeError>(
					std::format(
						"Parameters type must be equal to argument type. ({} != {})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						rightVal.getDetailTypeHold()
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& postfixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, std::move(rightVal))
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
			if (*lambdaFunction.getLambdaInfo().ParamsType == RuntimeBaseType::NodePointer)
				leftVal = NodePointer(leftVal.toNodeExpression());

			if (*lambdaFunction.getLambdaInfo().ParamsType != leftVal.getDetailTypeHold())
				return RuntimeError<RuntimeTypeError>(
					std::format(
						"Parameters type must be equal to argument type. ({} != {})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						leftVal.getDetailTypeHold()
					)
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& prefixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, std::move(leftVal))
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

		operationStack.pop();
	}

	if (!resultMap.contains(rootNodeExpression))
		return std::runtime_error("failed.");

	return resultMap[rootNodeExpression].value();
}

#endif //RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA