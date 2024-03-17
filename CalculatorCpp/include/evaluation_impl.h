#ifndef EVALUATION_IMPL_H
#define EVALUATION_IMPL_H

#include <iostream>
#include <cmath>
#include <stack>
#include <cctype>
#include <type_traits>
#include "parser.h"
#include "evaluation.h"


template<std::floating_point Floating>
Evaluate<Floating>::Evaluate(const Parser& parser) : parser{ parser } {}


template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating, Floating)>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Infix)
		throw std::runtime_error("EvaluationError: operator " + operatorLexeme + " is not consist in infix operators list.");
	mInfixOperatorFunctions[operatorLexeme] = operatorDefinition;
}

template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating)>& operatorDefinition) {
	if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Prefix)
		mPrefixOperatorFunctions[operatorLexeme] = operatorDefinition;
	else if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Postfix)
		mPostfixOperatorFunctions[operatorLexeme] = operatorDefinition;
	else
		throw std::runtime_error("EvaluationError: operator " + operatorLexeme + " is not consist in prefix or postfix operators list.");
}

template<std::floating_point Floating>
Floating Evaluate<Floating>::evaluateExpressionTree(Parser::Node* root) const {
	if (root == nullptr)
		throw std::runtime_error("EvaluationError: Failed to evaluate expression.");

	// base case if a leaf node.
	if (root->left == nullptr && root->right == nullptr)
		return std::stoi(root->value);

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Infix) {
		Floating leftVal = evaluateExpressionTree(root->left);
		Floating rightVal = evaluateExpressionTree(root->right);
		return evaluateInfix(root->value, leftVal, rightVal);
	}

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Postfix) {
		Floating rightVal = evaluateExpressionTree(root->right);
		return evaluatePostfix(root->value, rightVal);
	}

	if (parser.getOperatorType(root->value) == Parser::OperatorEvalType::Prefix) {
		Floating leftVal = evaluateExpressionTree(root->left);
		return evaluatePrefix(root->value, leftVal);
	}

	throw std::runtime_error("EvaluationError: Failed to evaluate expression.");
}

template<std::floating_point Floating>
Floating Evaluate<Floating>::evaluateInfix(const Parser::OperatorLexeme& opr, Floating left, Floating right) const {
	if (!mInfixOperatorFunctions.contains(opr))
		throw std::runtime_error("EvaluationError: Definition for operator " + opr + " not found.");
	return mInfixOperatorFunctions.at(opr)(left, right);
}

template<std::floating_point Floating>
Floating Evaluate<Floating>::evaluatePostfix(const Parser::OperatorLexeme& opr, Floating right) const {
	if (!mPostfixOperatorFunctions.contains(opr))
		throw std::runtime_error("EvaluationError: Definition for operator " + opr + " not found.");
	return mPostfixOperatorFunctions.at(opr)(right);
}

template<std::floating_point Floating>
Floating Evaluate<Floating>::evaluatePrefix(const Parser::OperatorLexeme& opr, Floating left) const {
	if (!mPrefixOperatorFunctions.contains(opr))
		throw std::runtime_error("EvaluationError: Definition for operator " + opr + " not found.");
	return mPrefixOperatorFunctions.at(opr)(left);
}

#endif // EVALUATION_IMPL_H