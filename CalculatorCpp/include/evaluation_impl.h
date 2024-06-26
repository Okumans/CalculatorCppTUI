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


inline Evaluate::Evaluate(const Parser& parser) :
	parser{ parser },
	mOperatorFunctions{} {}


inline Evaluate::Evaluate(const Parser& parser, const Evaluate& other) :
	parser{ parser },
	mOperatorFunctions{ other.mOperatorFunctions } {}


inline void Evaluate::addOperatorFunction(const Lambda& operatorDefinition) {
	std::string functionSignature(operatorDefinition.getLambdaSignature().value()); // handle error

	if (!parser.isOperator(functionSignature))
		throw RuntimeError<EvaluationDefinitionError>(std::format("The operator {} isn't consist in operators list.", functionSignature));

	if (static_cast<int8_t>(operatorDefinition.getNotation()) != static_cast<int8_t>(parser.getOperatorType(functionSignature)))
		throw RuntimeError<EvaluationDefinitionError>(std::format("The operator {} parser OperatorEvalType and Lambda function Notation Type are not the same.", functionSignature));

	mOperatorFunctions.emplace(functionSignature, operatorDefinition);
}

inline void Evaluate::addOperatorFunction(Lambda&& operatorDefinition) {
	std::string functionSignature(operatorDefinition.getLambdaSignature().value());

	if (!parser.isOperator(functionSignature))
		throw RuntimeError<EvaluationDefinitionError>(std::format("The operator {} isn't consist in operators list.", functionSignature));

	if (static_cast<int8_t>(operatorDefinition.getNotation()) != static_cast<int8_t>(parser.getOperatorType(functionSignature)))
		throw RuntimeError<EvaluationDefinitionError>(std::format("The operator {} parser OperatorEvalType and Lambda function Notation Type are not the same.", functionSignature));

	mOperatorFunctions.emplace(functionSignature, std::move(operatorDefinition));
}


inline Result<RuntimeTypedExprComponent> Evaluate::evaluateExpressionTree(const std::vector<NodeFactory::NodePos>& roots) const {

	Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error> evaluationResult{ Lambda::_NodeExpressionsEvaluator(roots, mOperatorFunctions) };
	
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

inline const std::unordered_map<Parser::Lexeme, Lambda>& Evaluate::getEvaluationLambdaFunction() const {
	return mOperatorFunctions;
}

#endif // EVALUATION_IMPL_H