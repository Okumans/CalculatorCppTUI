#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_LAMBDA

#include <sstream>
#include <format>
#include <variant>
#include "runtimeTypedExprComponent.h"

inline Lambda::Lambda(const Lambda& other) :
	mLambdaNotation{ other.mLambdaNotation },
	mLambdaInfo{ other.mLambdaInfo },
	mLambdaFunction{ other.mLambdaFunction },
	mLambdaFunctionSignature{ other.mLambdaFunctionSignature },
	BaseRuntimeTypedExprComponent(other.mType, other.mNodeExpression) {}

inline Lambda& Lambda::operator=(const Lambda& other) {
	if (this != &other) {
		mType = other.mType;
		mLambdaNotation = other.mLambdaNotation;
		mLambdaInfo = other.mLambdaInfo;
		mLambdaFunction = other.mLambdaFunction;
		mLambdaFunctionSignature = other.mLambdaFunctionSignature;
	}
	return *this;
}

inline Lambda::Lambda(Lambda&& other) noexcept
	: mLambdaInfo(std::move(other.mLambdaInfo)),
	mLambdaNotation(other.mLambdaNotation),
	mLambdaFunction(std::move(other.mLambdaFunction)),
	mLambdaFunctionSignature(std::move(other.mLambdaFunctionSignature)),
	BaseRuntimeTypedExprComponent(
		std::move(other.mType),
		other.mNodeExpression) { }

inline Lambda& Lambda::operator=(Lambda&& other) noexcept {
	if (this != &other) {
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
	mLambdaNotation{ lambdaNotation },
	mLambdaInfo{ RuntimeCompoundType::getLambdaInfo(lambdaType) },
	mLambdaFunction{ std::make_shared<std::function<RuntimeTypedExprComponent(LambdaArguments)>>(lambdaFunction) },
	mLambdaFunctionSignature{ lambdaFunctionSignature },
	BaseRuntimeTypedExprComponent(lambdaType, generateExpressionTree(lambdaFunctionSignature))
{
	// gurantree noexcept, assertion will be test in wrapper function
	// assert(mmLambdaInfo.ParamsNumbers == mLambdaParametersName.size());
	// return type will be check at runtime.
}

inline Lambda::Lambda(
	const RuntimeCompoundType& lambdaType,
	LambdaNotation lambdaNotation,
	NodePos lambdaFunctionRootNode) :
	mLambdaNotation{ lambdaNotation },
	mLambdaInfo{ RuntimeCompoundType::getLambdaInfo(lambdaType) },
	mLambdaFunction{ lambdaFunctionRootNode },
	BaseRuntimeTypedExprComponent(lambdaType, lambdaFunctionRootNode)
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
		return RuntimeTypeError(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", RuntimeType(lambdaType)), "Lambda::fromFunction");
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
		return RuntimeTypeError(std::format("Lambda Infix LambdaNotation must be a Storage with 2 argument. (cannot be \"{}\")", RuntimeType(lambdaType)), "Lambda::fromFunction");

	const RuntimeType lambdaReturnType{ RuntimeCompoundType::_getLambdaReturnType(lambdaType) };
	size_t lambdaParamsNumbers{ RuntimeCompoundType::_getLambdaParamsNumbers(lambdaType) };

	if (testArgument.size() == lambdaParamsNumbers) {
		try {
			RuntimeTypedExprComponent returnValue = lambdaFunction(testArgument);
			RuntimeType returnType = returnValue.getDetailTypeHold();

			if (returnType == lambdaReturnType)
				return Lambda(lambdaFunctionSignature, lambdaType, lambdaNotation, lambdaFunction);
			return RuntimeTypeError(
				std::format("Cannot construct Lambda, Parameter type not matched. ({}!={}))",
					lambdaReturnType,
					returnType
				),
				"Lambda::fromFunction"
			);
		}
		catch (const std::exception& e) {
			return LambdaEvaluationError(
				std::format("The lambda evaluation caused an exception \"{}\" during construction.", e.what()),
				"Lambda::fromFunction::mLambdaFunction");
		}
	}

	else
		return RuntimeTypeError(
			std::format("Cannot construct Lambda, Parameter amount not matched. ({}!={})",
				lambdaParamsNumbers, testArgument.size()),
			"Lambda::fromFunction"
		);
}

inline Result<Lambda, std::runtime_error> Lambda::fromExpressionNode(
	NodePos lambdaFunctionRootNode,
	const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions)
{
	if (!NodeFactory::validNode(lambdaFunctionRootNode))
		return RuntimeTypeError("The conversion from NodeExpression to Lambda failed due to an invalid NodeExpression.",
			"Lambda::fromExpressionNode");

	if (NodeFactory::node(lambdaFunctionRootNode).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
		Result<RuntimeType, std::runtime_error>&& returnTypeRaw{ getReturnType(lambdaFunctionRootNode, EvaluatorLambdaFunctions) };

		if (returnTypeRaw.isError())
			return RuntimeTypeError(
				returnTypeRaw.getException(),
				std::format("While determining type of lambdaFunctionRootNode \"{}\"",
					NodeFactory::node(lambdaFunctionRootNode).value),
				"Lambda::fromExpressionNode");

		std::vector<RuntimeType> parameterTypes;
		parameterTypes.reserve(NodeFactory::node(lambdaFunctionRootNode).utilityStorage.size());
		for (const std::pair<std::string, RuntimeType>& parameterWithType : NodeFactory::node(lambdaFunctionRootNode).utilityStorage)
			parameterTypes.emplace_back(parameterWithType.second);

		return Lambda(
			RuntimeCompoundType::Lambda(
				returnTypeRaw.moveValue(),
				parameterTypes.size() == 1 ?
				parameterTypes[0] :
				RuntimeCompoundType::Storage(parameterTypes)),
			LambdaNotation::Postfix,
			lambdaFunctionRootNode
		);
	}

	else if (NodeFactory::node(lambdaFunctionRootNode).nodestate == NodeFactory::Node::NodeState::Operator) {
		if (EvaluatorLambdaFunctions.contains(NodeFactory::node(lambdaFunctionRootNode).value))
			return EvaluatorLambdaFunctions.at(NodeFactory::node(lambdaFunctionRootNode).value);

		return LambdaConstructionError(
			std::format("The LambdaSignature \"{}\" not found in EvaluatorLambdaFunctions",
				NodeFactory::node(lambdaFunctionRootNode).value),
			"Lambda::fromExpressionNode");
	}
	return LambdaConstructionError(
		std::format(R"(NodeExpression NodeState should be LambdaFunction("1") or Operator("3") not "{}" (as a number))",
			static_cast<int>(NodeFactory::node(lambdaFunctionRootNode).nodestate)),
		"Lambda::fromExpressionNode");
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
	ss << mType;
	return ss.str();
}

inline NodeFactory::NodePos Lambda::generateExpressionTree(const std::string& functionSignature) const {
	const NodePos operatorNode{ NodeFactory::create(functionSignature) };
	NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;
	return operatorNode;
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> Lambda::evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const LambdaArguments& arguments) const {
	if (arguments.size() != mLambdaInfo.ParamsNumbers)
		return RuntimeTypeError(
			std::format(
				"Lambda parameter and argument amount not matched. ({} != {})",
				mLambdaInfo.ParamsNumbers,
				arguments.size()
			),
			"Lambda::evaluate"
		);

	if (arguments.size() == 1) {
		if (*mLambdaInfo.ParamsType != arguments[0].getDetailTypeHold())
			return RuntimeTypeError(
				std::format(
					"Lambda parameter and argument type not matched ({} != {}).",
					*mLambdaInfo.ParamsType,
					arguments[0].getDetailTypeHold()
				),
				"Lambda::evaluate"
			);
	}

	else if (mLambdaInfo.ParamsNumbers && !_fastCheckRuntimeTypeArgumentsType(*mLambdaInfo.ParamsType, arguments))
		return RuntimeTypeError(
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
			return LambdaEvaluationError(
				std::format("The lambda evaluation caused an exception \"{}\" during runtime.", e.what()),
				"Lambda::evaluate");
		}
	}

	std::unordered_map<std::string, Lambda> evaluatorLambdaFunctionsSnapshot(EvaluatorLambdaFunctions);
	const std::vector<std::pair<std::string, RuntimeType>> parameters{
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
			return LambdaConstructionError(
				tempFunc.getException(),
				std::format(
					"When attempting to create a constant lambda function \"{}\", which is used for determine lambda function evaluation result .",
					parameters[ind].first
				),
				"Lambda::evaluate"
			);

		evaluatorLambdaFunctionsSnapshot.try_emplace(parameters[ind].first, tempFunc.moveValue());
	}

	Result<RuntimeTypedExprComponent, std::runtime_error>&& res{
		_NodeExpressionEvaluate(
			NodeFactory::node(std::get<NodePos>(mLambdaFunction)).leftPos,
			evaluatorLambdaFunctionsSnapshot
		)
	};

	if (res.isError())
		return LambdaEvaluationError(
			res.getException(),
			std::format(
				"When attempting to evaluate lambda function on node \"{}\" (value={})",
				NodeFactory::node(std::get<NodePos>(mLambdaFunction)).leftPos,
				NodeFactory::validNode(NodeFactory::node(std::get<NodePos>(mLambdaFunction)).leftPos)
				? NodeFactory::node(std::get<NodePos>(mLambdaFunction)).leftNode().value
				: "NULL"
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

		NodeFactory::Node *currNode{ &NodeFactory::node(currNodePos) };

		// if currNode is a leaf node.
		if (!NodeFactory::validNode(currNode->rightPos) &&
			!NodeFactory::validNode(currNode->leftPos)) {
			if (currNode->value == ".")
				resultMap[currNodePos] = 0;

			else if (EvaluatorLambdaFunctions.contains(currNode->value) &&
				EvaluatorLambdaFunctions.at(currNode->value).getNotation() == Lambda::LambdaNotation::Constant) {
				Lambda constOperator{ EvaluatorLambdaFunctions.at(currNode->value) };
				Result<RuntimeTypedExprComponent, std::runtime_error>&& constOperatorResult{ constOperator.evaluate(EvaluatorLambdaFunctions, {}) };
				
				currNode = &NodeFactory::node(currNodePos); // update currNode, evaluate can change currNode address.

				if (constOperatorResult.isError())
					return LambdaEvaluationError(
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
			const std::vector<std::pair<std::string, RuntimeType>> *parameters{ &currNode->utilityStorage };
			const size_t parametersSize{ parameters->size() };
			std::vector<RuntimeTypedExprComponent> arguments;

			NodeFactory::NodePos currArgNodePos = currNode->rightPos;
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<RuntimeTypedExprComponent, std::runtime_error>&& result{
					_NodeExpressionEvaluate(
						NodeFactory::node(currArgNodePos).leftPos,
						EvaluatorLambdaFunctions
					)

				};

				currNode = &NodeFactory::node(currNodePos); // update currNode, _NodeExpressionEvaluate can change currNode address.
				parameters = &currNode->utilityStorage; // update parameters, _NodeExpressionEvaluate can change currNode address affects parameters.

				if (result.isError())
					return LambdaEvaluationError(
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

			if (arguments.empty()) {
				Result<Lambda, std::runtime_error> currLambda{ fromExpressionNode(currNodePos, EvaluatorLambdaFunctions) };
				if (currLambda.isError())
					return LambdaEvaluationError(
						currLambda.getException(),
						std::format(
							"When trying to convert nodeExpression to Lambda Function. (nodeExpression value = {})",
							currNodePos
						),
						"Lambda::_NodeExpressionEvaluate"
					);
				return { currLambda.moveValue() };
			}

			if (arguments.size() != parametersSize)
				return RuntimeTypeError(
					std::format(
						"Parameters size must be equal to argument size! ({}!={}).",
						arguments.size(),
						parametersSize
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			std::unordered_map<std::string, Lambda> tempEvaluatorLambdaFunctions(EvaluatorLambdaFunctions);

			for (size_t ind{ 0 }, len{ parameters->size() }; ind < len; ind++) {
				if ((*parameters)[ind].second != arguments[ind].getDetailTypeHold())
					return RuntimeTypeError(
						std::format(
							"parameters type must be same as argument type! ({}!={})",
							(*parameters)[ind].second,
							arguments[ind].getDetailTypeHold()
						),
						"Lambda::_NodeExpressionEvaluate"
					);

				RuntimeTypedExprComponent&& constLambdaValue{ std::move(arguments[ind]) };
				auto constLambda = [&constLambdaValue](const Lambda::LambdaArguments&) {
					return constLambdaValue;
					};

				Result<Lambda, std::runtime_error> tempFunc{
					Lambda::fromFunction(
						(*parameters)[ind].first,
						RuntimeCompoundType::Lambda(
							(*parameters)[ind].second,
							RuntimeBaseType::_Storage
						),
						Lambda::LambdaNotation::Constant,
						constLambda
					)
				};

				if (tempFunc.isError())
					return LambdaConstructionError(
						tempFunc.getException(),
						std::format(
							"When attempting to create a constant lambda function \"{}\", which is used for determine lambda function evaluation result.",
							(*parameters)[ind].first
						),
						"Lambda::_NodeExpressionEvaluate"
					);

				tempEvaluatorLambdaFunctions.try_emplace((*parameters)[ind].first, tempFunc.moveValue());
			}

			Result<RuntimeTypedExprComponent, std::runtime_error>&& leftVal{ _NodeExpressionEvaluate(currNode->leftPos, tempEvaluatorLambdaFunctions) };

			if (leftVal.isError())
				return LambdaEvaluationError(
					leftVal.getException(),
					std::format(
						"When attempting to evaluate expession content of a lambda function. (value = {})",
						NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
						? NodeFactory::node(currArgNodePos).leftNode().value
						: "Null"),
					"Lambda::_NodeExpressionEvaluate"
				);

			resultMap[currNodePos] = leftVal.moveValue();
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
					return StorageEvaluationError(
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

			if (!((*parametersType)[0] == leftVal.getDetailTypeHold() &&
				(*parametersType)[1] == rightVal.getDetailTypeHold()))
				return RuntimeTypeError(
					std::format(
						"Parameters type must be equal to argument type. ({}!={})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						RuntimeType(
							RuntimeCompoundType::Storage({
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
				return LambdaEvaluationError(
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
			if (*lambdaFunction.getLambdaInfo().ParamsType != rightVal.getDetailTypeHold())
				return RuntimeTypeError(
					std::format(
						"Parameters type must be equal to argument type. ({}!={})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						rightVal.getDetailTypeHold()
					),
					"Lambda::_NodeExpressionEvaluate"
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& postfixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, std::move(rightVal))
			};

			if (postfixOperatorResult.isError())
				return LambdaEvaluationError(
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
			if (*lambdaFunction.getLambdaInfo().ParamsType != leftVal.getDetailTypeHold())
				return RuntimeTypeError(
					std::format(
						"Parameters type must be equal to argument type. ({}!={})",
						*lambdaFunction.getLambdaInfo().ParamsType,
						leftVal.getDetailTypeHold()
					)
				);

			Result<RuntimeTypedExprComponent, std::runtime_error>&& prefixOperatorResult{
				lambdaFunction.evaluate(EvaluatorLambdaFunctions, std::move(leftVal))
			};

			if (prefixOperatorResult.isError())
				return LambdaEvaluationError(
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