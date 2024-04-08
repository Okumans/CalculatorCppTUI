#ifndef EVALUATION_IMPL_H
#define EVALUATION_IMPL_H

#include <iostream>
#include <cmath>
#include <stack>
#include <cctype>
#include <optional>
#include <type_traits>
#include <ranges>

#include "result.h"
#include "parser.h"
#include "evaluation.h"


template<std::floating_point Floating>
Evaluate<Floating>::Evaluate(const Parser& parser) : parser{ parser },
	mInfixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating, Floating)>>>() },
	mPostfixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>>>() },
	mPrefixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::OperatorLexeme, std::function<Floating(Floating)>>>() } {}

template<std::floating_point Floating>
Evaluate<Floating>::Evaluate(const Parser& parser, const Evaluate<Floating>& other) :
	parser{ parser },
	mConstantOperatorFunctions{ other.mConstantOperatorFunctions },
	mInfixOperatorFunctions{ other.mInfixOperatorFunctions },
	mPostfixOperatorFunctions{ other.mPostfixOperatorFunctions },
	mPrefixOperatorFunctions{ other.mPrefixOperatorFunctions } {}

template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating, Floating)>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Infix)
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in infix operators list.");
	(*mInfixOperatorFunctions)[operatorLexeme] = operatorDefinition;
}

template<std::floating_point Floating>
void Evaluate<Floating>::addOperatorFunction(const Parser::OperatorLexeme& operatorLexeme, const std::function<Floating(Floating)>& operatorDefinition) {
	if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Prefix)
		(*mPrefixOperatorFunctions)[operatorLexeme] = operatorDefinition;
	else if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Postfix)
		(*mPostfixOperatorFunctions)[operatorLexeme] = operatorDefinition;
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

	if (root->nodestate == Parser::Node::NodeState::LambdaFuntion) {
		std::vector<std::string>& parameters = root->utilityStorage;
		Parser pas(this->parser);
		Evaluate eval(pas, *this);


		std::vector<Floating> arguments;

		Parser::Node* curr = root->right;
		while (curr != nullptr) {
			Result<Floating> result = evaluateExpressionTree(curr->left);
			EXCEPT_RETURN(result);
			arguments.push_back(result.getValue());
			curr = curr->right;
		}

		if (arguments.size() != parameters.size())
			return 0;

		for (size_t ind{ 0 }, len{ parameters.size() }; ind < len; ind++)
		{
			eval.mConstantOperatorFunctions[parameters[ind]] = [ind, &arguments]() {return arguments[ind]; };
			pas.addOperatorEvalType(parameters[ind], Parser::OperatorEvalType::Constant);
		}

		Result<Floating> leftVal = eval.evaluateExpressionTree(root->left);
		EXCEPT_RETURN(leftVal);

		return leftVal.getValue();
	}

	if (root->nodestate == Parser::Node::NodeState::Storage) {
		std::vector<Floating> arguments;

		Parser::Node* curr = root->left;
		while (curr != nullptr) {
			Result<Floating> result = evaluateExpressionTree(curr->left);
			EXCEPT_RETURN(result);
			arguments.push_back(result.getValue());
			curr = curr->right;
		}

		if (arguments.size())
			return arguments.back();
		return EvaluationFailedError("Cannot evalutate noting.");
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
	if (!mInfixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mInfixOperatorFunctions->at(opr)(left, right);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluatePostfix(const Parser::OperatorLexeme& opr, Floating right) const {
	if (!mPostfixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPostfixOperatorFunctions->at(opr)(right);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluatePrefix(const Parser::OperatorLexeme& opr, Floating left) const {
	if (!mPrefixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPrefixOperatorFunctions->at(opr)(left);
}

template<std::floating_point Floating>
Result<Floating> Evaluate<Floating>::evaluateConstant(const Parser::OperatorLexeme& opr) const {
	if (!mConstantOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mConstantOperatorFunctions.at(opr)();
}

#endif // EVALUATION_IMPL_H