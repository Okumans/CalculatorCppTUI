#ifndef INITIALIZATION_IMPL_H
#define INITIALIZATION_IMPL_H

#include <cassert>
#include "initialization.h" // Include the header file to ensure correct template instantiation

#ifndef N_EVALUATE
template<typename Floating>
void initializeEvaluator(Evaluate<Floating>& evaluator) {
	evaluator.addOperatorFunction("+", [](Floating a, Floating b) {return a + b; });
	evaluator.addOperatorFunction("-", [](Floating a, Floating b) {return a - b; });
	evaluator.addOperatorFunction("*", [](Floating a, Floating b) {return a * b; });
	evaluator.addOperatorFunction("/", [](Floating a, Floating b) {return a / b; });
	evaluator.addOperatorFunction("//", [](Floating a, Floating b) {return std::floor(a / b); });
	evaluator.addOperatorFunction("^", [](Floating a, Floating b) {return std::pow(a, b); });
	evaluator.addOperatorFunction("!", [](Floating a) {return (Floating)std::tgamma(a + 1); });
	evaluator.addOperatorFunction("~", [](Floating a) {return (Floating)!(bool)a; });
	evaluator.addOperatorFunction("sqrt", [](Floating a) {return std::sqrt(a); });
	evaluator.addOperatorFunction("e+", [](Floating a, Floating b) {return a * std::pow(10, b); });
	evaluator.addOperatorFunction("k", [](Floating a) {return a * 1000; });
	evaluator.addOperatorFunction("ln", [](Floating a) {return std::log(a); });
	evaluator.addOperatorFunction("log2", [](Floating a) {return std::log2(a); });
	evaluator.addOperatorFunction("e", []() {return std::numbers::e; });
	evaluator.addOperatorFunction("pi", []() {return std::numbers::pi; });
	evaluator.addOperatorFunction("abs", [](Floating a) {return std::abs(a); });
}
#endif // N_EVALUATE

#if !defined(N_LEXER) && !defined(N_PARSER) && !defined(N_EVALUATE)
template<typename Floating>
void OperatorDefiner<Floating>::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating()>& operatorDefinition)
{
	assert(operatorEvalType == Parser::OperatorEvalType::Constant);
	mLexer.addKeyword(operatorName);
	mParser.addOperatorEvalType(operatorName, operatorEvalType);
	mParser.addOperatorLevel(operatorName, operatorLevel);
	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
}

template<typename Floating>
void OperatorDefiner<Floating>::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating)>& operatorDefinition)
{
	assert(operatorEvalType == Parser::OperatorEvalType::Postfix || operatorEvalType == Parser::OperatorEvalType::Prefix);
	mLexer.addKeyword(operatorName);
	mParser.addOperatorEvalType(operatorName, operatorEvalType);
	mParser.addOperatorLevel(operatorName, operatorLevel);
	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
}

template<typename Floating>
void OperatorDefiner<Floating>::defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating, Floating)>& operatorDefinition)
{
	assert(operatorEvalType == Parser::OperatorEvalType::Infix);
	mLexer.addKeyword(operatorName);
	mParser.addOperatorEvalType(operatorName, operatorEvalType);
	mParser.addOperatorLevel(operatorName, operatorLevel);
	mEvaluate.addOperatorFunction(operatorName, operatorDefinition);
}

#endif // N_LEXER && N_PARSER && N_EVALUATE
#endif // INITIALIZATION_IMPL_H
