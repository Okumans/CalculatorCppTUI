#pragma once

#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include "runtimeType.h"
#include "runtimeTypedExprComponent.h"
#include "result.h"
#include "parser.h"
#include "nodeFactory.h"

class Parser;

class EvaluationDefinitionError : public std::runtime_error {
public:
	explicit EvaluationDefinitionError(const std::string& msg) : std::runtime_error("EvaluationDefinitionError: " + msg) {}
};


// Custom exception class for runtime type errors
class EvaluationFailedError : public std::runtime_error {
public:
	// Constructor with a single message
	explicit EvaluationFailedError(const std::string& message)
		: std::runtime_error("EvaluationFailedError: " + message) {}

	// Constructor with message and origin information
	explicit EvaluationFailedError(const std::string& message, const std::string& from)
		: std::runtime_error("EvaluationFailedError: " + message + " (from: " + from + ")") {}

	// Constructor with chained error, message, and origin information
	explicit EvaluationFailedError(const std::runtime_error& baseError, const std::string& message, const std::string& from)
		: std::runtime_error("EvaluationFailedError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

class Evaluate {
private:
	const Parser& parser;
	std::unordered_map<Parser::Lexeme, Lambda> mOperatorFunctions;

public:
	Evaluate(const Parser& parser);
	Evaluate(const Parser& parser, const Evaluate& other);
	void addOperatorFunction(const Lambda& operatorDefinition);
	void addOperatorFunction(Lambda&& operatorDefinition);
	Result<RuntimeTypedExprComponent> evaluateExpressionTree(const std::vector<NodeFactory::NodePos>& roots) const;
	const std::unordered_map<Parser::Lexeme, Lambda>& getEvaluationLambdaFunction() const;
};

#include "evaluation_impl.h"