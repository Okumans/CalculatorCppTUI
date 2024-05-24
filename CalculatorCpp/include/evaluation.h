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


struct EvaluationDefinitionError {
	static const std::string prefix;
};
inline const std::string EvaluationDefinitionError::prefix = "EvaluationDefinitionError";

struct EvaluationFailedError {
	static const std::string prefix;
};
inline const std::string EvaluationFailedError::prefix = "EvaluationFailedError";

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