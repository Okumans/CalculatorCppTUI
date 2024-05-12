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

// summation lambdaFunction implementation
const auto summationLambdaFunction = [](const std::unordered_map<Parser::Lexeme, Lambda> &EvaluatorLambdaFunction) {
	return Lambda::fromFunction(
		"summation",																							// LambdaFunctionSignature	= "summation"
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



//const Result<Lambda, std::runtime_error> nStorageLambdaFunction = Lambda::fromFunction(
//	"nStorage",
//	RCT::Lambda(, Number)
//);


inline void initializeEvaluator(Evaluate& evaluator) {
	evaluator.addOperatorFunction(addLambdaFunction.getValue());
	evaluator.addOperatorFunction(subtractLambdaFunction.getValue());
	evaluator.addOperatorFunction(multiplyLambdaFunction.getValue());
	evaluator.addOperatorFunction(divideLambdaFunction.getValue());
	evaluator.addOperatorFunction(powerLambdaFunction.getValue());
	evaluator.addOperatorFunction(constELambdaFunction.getValue());
	evaluator.addOperatorFunction(sqrtLambdaFunction.getValue());
	evaluator.addOperatorFunction(summationLambdaFunction(evaluator.getEvaluationLambdaFunction()).getValue());
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
