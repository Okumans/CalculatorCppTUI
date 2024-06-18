#ifndef EVALUATION_IMPL_H
#define EVALUATION_IMPL_H

#include <iostream>
#include <cmath>
#include <stack>
#include <cctype>
#include <optional>
#include <type_traits>
#include <ranges>
#include <format>
#include "result.h"
#include "parser.h"
#include "evaluation.h"
#include "nodeFactory.h"


inline void Evaluate::addOperatorFunction(const Lambda& operatorDefinition) {
	std::string functionSignature(operatorDefinition.getLambdaSignature().value()); // handle error
	mOperatorFunctions.emplace(functionSignature, operatorDefinition);
}

inline void Evaluate::addOperatorFunction(Lambda&& operatorDefinition) {
	std::string functionSignature(operatorDefinition.getLambdaSignature().value());
	mOperatorFunctions.emplace(functionSignature, std::move(operatorDefinition));
}


inline Result<RuntimeTypedExprComponent> Evaluate::evaluateExpressionTree(const std::vector<NodeFactory::NodePos>& roots, const std::unordered_map<NodeFactory::NodePos, NodeFactory::NodePos> nodeDependency) {

	Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error> evaluationResult{ Lambda::_NodeExpressionsEvaluator(roots, mOperatorFunctions, nodeDependency) };
	
	if (evaluationResult.isError())
		return RuntimeError<EvaluationFailedError>(
			evaluationResult.getException(),
			std::format(
				"When trying to evaluate Expression tree. (nodeExpression value = {})",
				roots[0]
			),
			"Evaluate::evaluateExpressionTree"
		);
	
	return evaluationResult.getValue().back();
}

inline std::unordered_map<Parser::Lexeme, Lambda>& Evaluate::getEvaluationLambdaFunction() {
	return mOperatorFunctions;
}

#endif // EVALUATION_IMPL_H