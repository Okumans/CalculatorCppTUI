#pragma once
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include "result.h"
#include "parser.h"

class Parser;

class EvaluationDefinitionError : public std::runtime_error {
public:
	explicit EvaluationDefinitionError(const std::string& msg) : std::runtime_error("EvaluationDefinitionError: " + msg) {}
};

class EvaluationFailedError : public std::runtime_error {
public:
	explicit EvaluationFailedError(const std::string& msg) : std::runtime_error("EvaluationFailedError: " + msg) {}
};

template<std::floating_point Floating>
class Evaluate {
private:
	const Parser& parser;
	std::shared_ptr<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>>> mPrefixOperatorFunctions;
	std::shared_ptr<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating, Floating)>>> mInfixOperatorFunctions;
	std::shared_ptr<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>>> mPostfixOperatorFunctions;
	std::unordered_map<Parser::OperatorLexeme, std::function<Floating()>> mConstantOperatorFunctions;

public:
	Evaluate(const Parser& parser);
	Evaluate(const Parser& parser, const Evaluate& other);
	void addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating, Floating)>& operatorDefinition);
	void addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating)>& operatorDefinition);
	void addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating()>& operatorDefinition);
	Result<Floating> evaluateExpressionTree(Parser::Node* root) const;
private:
	Result<Floating> evaluatePrefix(const Parser::OperatorLexeme& opr, Floating left) const;
	Result<Floating> evaluateInfix(const Parser::OperatorLexeme& opr, Floating left, Floating right) const;
	Result<Floating> evaluatePostfix(const Parser::OperatorLexeme& opr, Floating right) const;
	Result<Floating> evaluateConstant(const Parser::OperatorLexeme& opr) const;
};

#include "evaluation_impl.h"