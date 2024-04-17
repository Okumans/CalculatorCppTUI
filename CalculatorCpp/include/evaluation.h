#pragma once
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include "result.h"
#include "parser.h"
#include "nodeFactory.h"

class Parser;

class EvaluationDefinitionError : public std::runtime_error {
public:
	explicit EvaluationDefinitionError(const std::string& msg) : std::runtime_error("EvaluationDefinitionError: " + msg) {}
};

class EvaluationFailedError : public std::runtime_error {
public:
	explicit EvaluationFailedError(const std::string& msg) : std::runtime_error("EvaluationFailedError: " + msg) {}
};

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <Arithmetic Number>
class Evaluate {
private:
	const Parser& parser;
	std::shared_ptr<std::unordered_map<Parser::Lexeme, std::function<Number(Number)>>> mPrefixOperatorFunctions;
	std::shared_ptr<std::unordered_map<Parser::Lexeme, std::function<Number(Number, Number)>>> mInfixOperatorFunctions;
	std::shared_ptr<std::unordered_map<Parser::Lexeme, std::function<Number(Number)>>> mPostfixOperatorFunctions;
	std::unordered_map<Parser::Lexeme, std::function<Number()>> mConstantOperatorFunctions;

public:
	Evaluate(const Parser& parser);
	Evaluate(const Parser& parser, const Evaluate& other);
	void addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number(Number, Number)>& operatorDefinition);
	void addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number(Number)>& operatorDefinition);
	void addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number()>& operatorDefinition);
	Result<Number> evaluateExpressionTree(NodeFactory::NodePos root) const;

private:
	Result<Number> evaluatePrefix(const Parser::Lexeme& opr, Number left) const;
	Result<Number> evaluateInfix(const Parser::Lexeme& opr, Number left, Number right) const;
	Result<Number> evaluatePostfix(const Parser::Lexeme& opr, Number right) const;
	Result<Number> evaluateConstant(const Parser::Lexeme& opr) const;
};

#include "evaluation_impl.h"