#ifndef EVALUATION_IMPL_H
#define EVALUATION_IMPL_H

#include <iostream>
#include <cmath>
#include <stack>
#include <cctype>
#include <optional>
#include <type_traits>
#include "result.h"
#include "parser.h"
#include "evaluation.h"


template<std::floating_point Floating>
Evaluate<Floating>::Evaluate(const Parser& parser) : parser{ parser } {}


template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating, Floating)>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Infix)
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in infix operators list.");
	mInfixOperatorFunctions[operatorLexeme] = operatorDefinition;
}

template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating)>& operatorDefinition) {
	if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Prefix)
		mPrefixOperatorFunctions[operatorLexeme] = operatorDefinition;
	else if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Postfix)
		mPostfixOperatorFunctions[operatorLexeme] = operatorDefinition;
	else
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in prefix or postfix operators list.");
}

template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating()>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Constant)
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in constants list.");
	mConstantOperatorFunctions[operatorLexeme] = operatorDefinition;
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluateExpressionTree(Parser::Node* root) const {
	if (root == nullptr)
		return EvaluationFailedError("Failed to evaluate expression.");

	// base case if a leaf node.
	if (root->left == nullptr && root->right == nullptr) {
		if (root->value == ".")
			return 0;
		else if (parser.isOperator(root->value) && parser.getOperatorType(root->value) == Parser::OperatorEvalType::Constant) {
			if (Result<Floating> res(evaluateConstant(root->value)); !res.isError())
				return res.getValue();
			else
				return res.getException();
		}
		return std::stod(root->value);
	}

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Infix) {
		Result<Floating> leftVal = evaluateExpressionTree(root->left);
		Result<Floating> rightVal = evaluateExpressionTree(root->right);

		if (leftVal.isError()) return leftVal.getException();
		if (rightVal.isError()) return rightVal.getException();

		if (Result<Floating> res(evaluateInfix(root->value, leftVal.getValue(), rightVal.getValue())); !res.isError())
			return res.getValue();
		else 
			return res.getException();
	}

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Postfix) {
		Result<Floating> rightVal = evaluateExpressionTree(root->right);
		if (rightVal.isError()) return rightVal.getException();

		if (Result<Floating> res(evaluatePostfix(root->value, rightVal.getValue())); !res.isError())
			return res.getValue();
		else
			return res.getException();
	}

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Prefix) {
		Result<Floating> leftVal = evaluateExpressionTree(root->left);
		if (leftVal.isError()) return leftVal.getException();

		if (Result<Floating> res(evaluatePrefix(root->value, leftVal.getValue())); !res.isError())
			return res.getValue();
		else
			return res.getException();
	}

	return EvaluationFailedError("Failed to evaluate expression.");
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluateInfix(const Parser::OperatorLexeme& opr, Floating left, Floating right) const {
	if (!mInfixOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mInfixOperatorFunctions.at(opr)(left, right);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluatePostfix(const Parser::OperatorLexeme& opr, Floating right) const {
	if (!mPostfixOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPostfixOperatorFunctions.at(opr)(right);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluatePrefix(const Parser::OperatorLexeme& opr, Floating left) const {
	if (!mPrefixOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPrefixOperatorFunctions.at(opr)(left);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluateConstant(const Parser::OperatorLexeme& opr) const {
	if (!mConstantOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mConstantOperatorFunctions.at(opr)();
}

#endif // EVALUATION_IMPL_H