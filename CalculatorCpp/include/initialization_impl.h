#ifndef INITIALIZATION_IMPL_H
#define INITIALIZATION_IMPL_H

#include <cassert>
#include <numbers>

#include "initialization.h"
#include "runtimeTypedExprComponent.h"

#ifndef N_EVALUATE1

using RCT = RuntimeCompoundType;

// addition lambdaFunction implementation
const Result<Lambda, std::runtime_error> addLambdaFunction = Lambda::fromFunction(
	"+",																									// LambdaFunctionSignature	= "+"
	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number
		return Number(args[0].getNumber() + args[1].getNumber());
	}
);

// subtraction lambdaFunction implementation
const Result<Lambda, std::runtime_error> subtractLambdaFunction = Lambda::fromFunction(
	"-",																									// LambdaFunctionSignature	= "-"
	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number
		return Number(args[0].getNumber() - args[1].getNumber());
	}
);

// multiplication lambdaFunction implementation
const Result<Lambda, std::runtime_error> multiplyLambdaFunction = Lambda::fromFunction(
	"*",																									// LambdaFunctionSignature	= "*"
	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number
		return Number(args[0].getNumber() * args[1].getNumber());
	}
);

// division lambdaFunction implementation
const Result<Lambda, std::runtime_error> divideLambdaFunction = Lambda::fromFunction(
	"/",																									// LambdaFunctionSignature	= "/"
	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number
		return Number(args[0].getNumber() / args[1].getNumber());
	}
);

// power lambdaFunction implementation
const Result<Lambda, std::runtime_error> powerLambdaFunction = Lambda::fromFunction(
	"^",																									// LambdaFunctionSignature	= "^"
	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number
		return Number(std::powl(args[0].getNumber(), args[1].getNumber()));
	}
);

// e constant lambdaFunction implementation
const Result<Lambda, std::runtime_error> constELambdaFunction = Lambda::fromFunction(
	"e",																									// LambdaFunctionSignature	= "e"
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::_Storage),										// LambdaType				= Lambda[Number, Storage_NULL]
	Lambda::LambdaNotation::Constant,																		// LambdaNotation			= Lambda::LambdaNotation::Constant
	[](const Lambda::LambdaArguments&) -> RuntimeTypedExprComponent {										// LambdaFunction			= () -> Number
		return Number(std::numbers::e);
	}
);

// square root lambdaFunction implementation
const Result<Lambda, std::runtime_error> sqrtLambdaFunction = Lambda::fromFunction(
	"sqrt",																									// LambdaFunctionSignature	= "sqrt"
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::Number),											// LambdaType				= Lambda[Number, Number]
	Lambda::LambdaNotation::Postfix,																		// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number) -> Number
		return Number(std::sqrtl(args[0].getNumber()));
	}
);

// abs lambdaFunction implementation
const Result<Lambda, std::runtime_error> absLambdaFunction = Lambda::fromFunction(
	"sqrt",																									// LambdaFunctionSignature	= "abs"
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::Number),											// LambdaType				= Lambda[Number, Number]
	Lambda::LambdaNotation::Postfix,																		// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number) -> Number
		return Number(std::fabsl(args[0].getNumber()));
	}
);

// summation lambdaFunction implementation
const auto sigmaLambdaFunction = [](const std::unordered_map<Parser::Lexeme, Lambda>& EvaluatorLambdaFunction) {
	return Lambda::fromFunction(
		"sigma",																							// LambdaFunctionSignature	= "sigma"
		RCT::Lambda(
			RuntimeBaseType::Number,
			RCT::Storage(
				RCT::Storage(
					RuntimeBaseType::Number,
					RuntimeBaseType::Number
				),
				RCT::Lambda(
					RuntimeBaseType::Number,
					RuntimeBaseType::Number
				)
			)
		),																										// LambdaType				= Lambda[Number, Storage[Storage[Number, Number], Lambda[Number, Number]]]
		Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
		[&EvaluatorLambdaFunction](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {			// LambdaFunction			= ([0]: Storage[Number, Number], [1]: Lambda[Number, Number]) -> Number
			int64_t start{ static_cast<int64_t>(args[0].getStorage()[0].getNumber()) };
			int64_t stop{ static_cast<int64_t>(args[0].getStorage()[1].getNumber()) };

			long double summation{ 0 };
			const Lambda& calcFunction{ args[1].getLambda() };
			for (; start < stop; start++)
				summation += calcFunction.evaluate(EvaluatorLambdaFunction, static_cast<long double>(start)).getValue().getNumber();

			return Number(summation);
		}
	);
	};

const Result<Lambda, std::runtime_error> sumLambdaFunction = Lambda::fromFunction(
	"sum",																										// LambdaSignature			= "sum"
	RCT::Lambda(RuntimeEvaluate::RuntimeEvaluate, RCT::Storage(RuntimeBaseType::Number)),						// LambdaType				= Lambda[Lambda[Number, Storage[Number...]], Number]
	Lambda::LambdaNotation::Postfix,																			// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {										// LambdaFunction			= ([0]: Number) -> Lambda[Lamba[Number, Number], Storage[Number...]]
		return Lambda::fromFunction("", // null signature (does not add EvaluatorLambdaFunction)				// LambdaSignature			= _
		RCT::Lambda(																							// LambdaType				= Lambda[Lamba[Number, Number], Storage[Number...]]
			RuntimeBaseType::Number,
			RCT::gurantreeNoRuntimeEvaluateStorage(
				std::vector<RuntimeType>(
					static_cast<size_t>(args[0].getStorage()[0].getNumber()),
					RuntimeBaseType::Number)
			)
		),
			Lambda::LambdaNotation::Postfix,																	// LambdaNotation			= Lambda::LambdaNotation::Postfix
			[](const Lambda::LambdaArguments& _args) -> RuntimeTypedExprComponent {
				long double summation{ 0 };
				for (const RuntimeTypedExprComponent& _arg : _args)
					summation += _arg.getNumber();
				return Number(summation);
			}
).getValue();
	}
);

const Result<Lambda, std::runtime_error> indexLambdaFunction = Lambda::fromFunction(
	"index",																									// LambdaSignature			= "index"
	RCT::Lambda(RuntimeEvaluate::RuntimeEvaluate, RCT::Storage(RuntimeBaseType::Number)),						// LambdaType				= Lambda[Lambda[Lamba[Number, Number], Storage[Number...]], Number]
	Lambda::LambdaNotation::Postfix,																			// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {										// LambdaFunction			= ([0]: Number) -> Lambda[Lamba[Number, Number], Storage[Number...]]
		return Lambda::fromFunction("", // null signature (does not add EvaluatorLambdaFunction)				// LambdaSignature			= _
		RCT::Lambda(																							// LambdaType				= Lambda[Lamba[Number, Number], Storage[Number...]]
			RuntimeEvaluate::RuntimeEvaluate,
			RCT::Storage(
				RCT::gurantreeNoRuntimeEvaluateStorage(
					std::vector<RuntimeType>(
						static_cast<size_t>(args[0].getStorage()[0].getNumber()),
						RuntimeBaseType::Number)
				),
				RCT::Storage(RuntimeBaseType::Number)
			)
		),
			Lambda::LambdaNotation::Infix,																	// LambdaNotation			= Lambda::LambdaNotation::Postfix
			[](const Lambda::LambdaArguments& _args) -> RuntimeTypedExprComponent {								// LambdaFunction			= ([0]: Storage[Number...]) -> Lamba[Number, Number]															// LambdaNotation			= Lambda::LambdaNotation::Postfix
				size_t index{ static_cast<size_t>(_args[1].getStorage()[0].getNumber()) };
				if (index >= _args[0].getStorage().size())
					return Number(NodeFactory::NodePosNull);
				return _args[0][index].getNumber();
			}
).getValue(); // gurantree valid result
	}
);

//const Result<Lambda, std::runtime_error> forLoopFunction = Lambda::fromFunction(
//	"for ",																									// LambdaFunctionSignature	= "for"
//	RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
//	Lambda::LambdaNotation::Infix,																			// LambdaNotation			= Lambda::LambdaNotation::Infix
//	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number, [1]: Number) -> Number (void)
//		memory[static_cast<size_t>(args[0].getNumber())] = args[1].getNumber();
//		return Number(0);
//	}
//);

static std::unordered_map<size_t, std::optional<RuntimeTypedExprComponent>> memory;
const auto assignNumberFunction = [](const std::unordered_map<Parser::Lexeme, Lambda>& EvaluatorLambdaFunction) {
	return Lambda::fromFunction(
		":=",																										// LambdaFunctionSignature	= ":="
		RCT::Lambda(RuntimeBaseType::Number, RCT::Storage(RuntimeBaseType::Number, RuntimeBaseType::NodePointer)),	// LambdaType				= Lambda[Number, Storage[Number, Number]]
		Lambda::LambdaNotation::Infix,																				// LambdaNotation			= Lambda::LambdaNotation::Infix
		[&EvaluatorLambdaFunction](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {										// LambdaFunction			= ([0]: Number, [1]: Number) -> Number (void)
			Result<RuntimeTypedExprComponent, std::runtime_error> pointedObjectResult{ args[1].getNodePointer().getPointed(EvaluatorLambdaFunction) };

			if (pointedObjectResult.isError())
				throw pointedObjectResult.getException();

			memory[static_cast<size_t>(args[0].getNumber())] = pointedObjectResult.moveValue();
			return Number(0);
		}
	);
};

const Result<Lambda, std::runtime_error> getNumberFunction = Lambda::fromFunction(
	"@",																									// LambdaFunctionSignature	= "@"
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::Number),											// LambdaType				= Lambda[Number, Number]
	Lambda::LambdaNotation::Postfix,																		// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number) -> Number
		return memory[static_cast<size_t>(args[0].getNumber())].value_or(Number(0));
	}
);

const Result<Lambda, std::runtime_error> factorFunction = Lambda::fromFunction(
	"factor",																									// LambdaFunctionSignature	= "@"
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::Number),											// LambdaType				= Lambda[Number, Number]
	Lambda::LambdaNotation::Postfix,																		// LambdaNotation			= Lambda::LambdaNotation::Postfix
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {									// LambdaFunction			= ([0]: Number) -> Number
		int64_t number{ static_cast<int64_t>(args[0].getNumber()) };
		std::vector<RuntimeTypedExprComponent> factors;
		int64_t sqrtNumber = static_cast<int64_t>(std::sqrtl(static_cast<long double>(number)));

		for (int64_t i = 1; i <= sqrtNumber; ++i) {
			if (number % i == 0) {
				factors.emplace_back(i);
				if (i != number / i)
					factors.emplace_back(number / i);
			}
		}

		return Storage::fromVector(std::move(factors));
	}
);

const Result<Lambda, std::runtime_error> andLambdaFunction = Lambda::fromFunction(
	"&&",
	RCT::Lambda(RuntimeBaseType::Number, RuntimeCompoundType::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),
	Lambda::LambdaNotation::Infix,
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {
		return Number(static_cast<bool>(args[0].getNumber()) && static_cast<bool>(args[1].getNumber()));
	}
);

const Result<Lambda, std::runtime_error> orLambdaFunction = Lambda::fromFunction(
	"||",
	RCT::Lambda(RuntimeBaseType::Number, RuntimeCompoundType::Storage(RuntimeBaseType::Number, RuntimeBaseType::Number)),
	Lambda::LambdaNotation::Infix,
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {
		return Number(static_cast<bool>(args[0].getNumber()) || static_cast<bool>(args[1].getNumber()));
	}
);

static bool isSameTree(NodeFactory::NodePos p, NodeFactory::NodePos q) {
	if (!NodeFactory::validNode(p) && !NodeFactory::validNode(q)) return true; // Both are null
	if (!NodeFactory::validNode(p) || !NodeFactory::validNode(q)) return false; // One is null, the other is not

	const NodeFactory::Node& nodeP{ NodeFactory::node(p) };
	const NodeFactory::Node& nodeQ{ NodeFactory::node(q) };

	if (nodeP != nodeQ) return false; // Values are different
	return isSameTree(nodeP.leftPos, nodeQ.leftPos) && isSameTree(nodeP.rightPos, nodeQ.rightPos); // Recursively check left and right children
}

const Result<Lambda, std::runtime_error> equalityLambdaFunction = Lambda::fromFunction(
	"==",
	RCT::Lambda(RuntimeBaseType::Number, RuntimeCompoundType::Storage(RuntimeBaseType::NodePointer, RuntimeBaseType::NodePointer)),
	Lambda::LambdaNotation::Infix,
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {
		return isSameTree(args[0].getNodePointer().getPointerIndex(), args[1].getNodePointer().getPointerIndex());
	}
);

const Result<Lambda, std::runtime_error> lengthLambdaFunction = Lambda::fromFunction(
	"@size",
	RCT::Lambda(RuntimeBaseType::Number, RuntimeBaseType::NodePointer),
	Lambda::LambdaNotation::Prefix,
	[](const Lambda::LambdaArguments& args) -> RuntimeTypedExprComponent {
		if (!args[0].getNodePointer().isNodePointerValid())
			return Number(0);

		if (const NodeFactory::Node& pointerNode{ args[0].getNodePointer().getPointerNode() };
			pointerNode.nodestate == NodeFactory::Node::NodeState::LambdaFuntion ||
			pointerNode.nodestate == NodeFactory::Node::NodeState::Storage) {
			size_t size{ 1 };
			NodeFactory::NodePos currNodePos{ pointerNode.rightPos };
			while (NodeFactory::validNode(currNodePos)) {
				size++;
				currNodePos = NodeFactory::node(currNodePos).rightPos;
			}

			return Number(static_cast<long double>(size));
		}

		return Number(1);
	}
);

inline void initializeEvaluator(Evaluate& evaluator) {
	evaluator.addOperatorFunction(addLambdaFunction.getValue());
	evaluator.addOperatorFunction(subtractLambdaFunction.getValue());
	evaluator.addOperatorFunction(multiplyLambdaFunction.getValue());
	evaluator.addOperatorFunction(divideLambdaFunction.getValue());
	evaluator.addOperatorFunction(powerLambdaFunction.getValue());
	evaluator.addOperatorFunction(constELambdaFunction.getValue());
	evaluator.addOperatorFunction(sqrtLambdaFunction.getValue());
	evaluator.addOperatorFunction(sigmaLambdaFunction(evaluator.getEvaluationLambdaFunction()).getValue());
	evaluator.addOperatorFunction(sumLambdaFunction.getValue());
	evaluator.addOperatorFunction(indexLambdaFunction.getValue());
	evaluator.addOperatorFunction(getNumberFunction.getValue());
	evaluator.addOperatorFunction(assignNumberFunction(evaluator.getEvaluationLambdaFunction()).getValue());
	evaluator.addOperatorFunction(factorFunction.getValue());
	evaluator.addOperatorFunction(andLambdaFunction.getValue());
	evaluator.addOperatorFunction(orLambdaFunction.getValue());
	evaluator.addOperatorFunction(equalityLambdaFunction.getValue());
	evaluator.addOperatorFunction(lengthLambdaFunction.getValue());
	/*evaluator.addOperatorFunction("+", [](Floating a, Floating b) {return a + b; });
	evaluator.addOperatorFunction("-", [](Floating a, Floating b) {return a - b; });
	evaluator.addOperatorFunction("*", [](Floating a, Floating b) {return a * b; });
	evaluator.addOperatorFunction("/", [](Floating a, Floating b) {return a / b; });
	evaluator.addOperatorFunction("//", [](Floating a, Floating b) {return std::floor(a / b); });
	evaluator.addOperatorFunction("^", [](Floating a, Floating b) {return std::pow(a, b); });
	evaluator.addOperatorFunction("!", [](Floating a) {return (Floating)std::tgamma(a + 1); });
	evaluator.addOperatorFunction("~", [](Floating a) {return (Floating)!(bool)a; });
	evaluator.addOperatorFunction("sqrt", [](Floating a) {return std::sqrt(a); });
	evaluator.addOperatorFunction("k", [](Floating a) {return a * 1000; });
	evaluator.addOperatorFunction("ln", [](Floating a) {return std::log(a); });
	evaluator.addOperatorFunction("log2", [](Floating a) {return std::log2(a); });
	evaluator.addOperatorFunction("e", []() {return std::numbers::e; });
	evaluator.addOperatorFunction("pi", []() {return std::numbers::pi; });
	evaluator.addOperatorFunction("abs", [](Floating a) {return std::abs(a); });*/
}
#endif // N_EVALUATE

#if !defined(N_LEXER) && !defined(N_PARSER) && !defined(N_EVALUATE)

//void OperatorDefiner::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating()>& operatorDefinition)
//{
//	assert(operatorEvalType == Parser::OperatorEvalType::Constant);
//	mLexer.addKeyword(operatorName);
//	mParser.addOperatorEvalType(operatorName, operatorEvalType);
//	mParser.addOperatorLevel(operatorName, operatorLevel);
//	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
//}
//
//void OperatorDefiner::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating)>& operatorDefinition)
//{
//	assert(operatorEvalType == Parser::OperatorEvalType::Postfix || operatorEvalType == Parser::OperatorEvalType::Prefix);
//	mLexer.addKeyword(operatorName);
//	mParser.addOperatorEvalType(operatorName, operatorEvalType);
//	mParser.addOperatorLevel(operatorName, operatorLevel);
//	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
//}
//
//void OperatorDefiner::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating, Floating)>& operatorDefinition)
//{
//	assert(operatorEvalType == Parser::OperatorEvalType::Infix);
//	mLexer.addKeyword(operatorName);
//	mParser.addOperatorEvalType(operatorName, operatorEvalType);
//	mParser.addOperatorLevel(operatorName, operatorLevel);
//	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
//}

#endif // N_LEXER && N_PARSER && N_EVALUATE
#endif // INITIALIZATION_IMPL_H
