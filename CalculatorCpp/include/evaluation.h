#pragma once
#include "parser.h"
#include <unordered_map>
#include <functional>
#include <tuple>
#include <type_traits>

class Parser;

// todo: add EvaluateError class

template<std::floating_point Floating>
class Evaluate {
private:
	const Parser& parser;
	std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>> mPrefixOperatorFunctions;
	std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating, Floating)>> mInfixOperatorFunctions;
	std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>> mPostfixOperatorFunctions;

public:
	explicit Evaluate(const Parser& parser);
	void addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating, Floating)>& operatorDefinition);
	void addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating)>& operatorDefinition);
	Floating evaluateExpressionTree(Parser::Node* root) const;
private:
	Floating evaluatePrefix(const Parser::OperatorLexeme& opr, Floating left) const;
	Floating evaluateInfix(const Parser::OperatorLexeme& opr, Floating left, Floating right) const;
	Floating evaluatePostfix(const Parser::OperatorLexeme& opr, Floating right) const;
};

#include "evaluation_impl.h"