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

template <Arithmetic Number>
Evaluate<Number>::Evaluate(const Parser& parser) : parser{ parser },
mInfixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::Lexeme, std::function<Number(Number, Number)>>>() },
mPostfixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::Lexeme, std::function<Number(Number)>>>() },
mPrefixOperatorFunctions{ std::make_shared<std::unordered_map<Parser::Lexeme, std::function<Number(Number)>>>() } {}

template <Arithmetic Number>
Evaluate<Number>::Evaluate(const Parser& parser, const Evaluate<Number>& other) :
	parser{ parser },
	mConstantOperatorFunctions{ other.mConstantOperatorFunctions },
	mInfixOperatorFunctions{ other.mInfixOperatorFunctions },
	mPostfixOperatorFunctions{ other.mPostfixOperatorFunctions },
	mPrefixOperatorFunctions{ other.mPrefixOperatorFunctions } {}

template <Arithmetic Number>
void Evaluate<Number>::addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number(Number, Number)>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Infix)
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in infix operators list.");
	(*mInfixOperatorFunctions)[operatorLexeme] = operatorDefinition;
}

template <Arithmetic Number>
void Evaluate<Number>::addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number(Number)>& operatorDefinition) {
	if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Prefix)
		(*mPrefixOperatorFunctions)[operatorLexeme] = operatorDefinition;
	else if (parser.isOperator(operatorLexeme) && parser.getOperatorType(operatorLexeme) == Parser::OperatorEvalType::Postfix)
		(*mPostfixOperatorFunctions)[operatorLexeme] = operatorDefinition;
	else
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in prefix or postfix operators list.");
}

template <Arithmetic Number>
void Evaluate<Number>::addOperatorFunction(const Parser::Lexeme& operatorLexeme, const std::function<Number()>& operatorDefinition) {
	if (!parser.isOperator(operatorLexeme) || parser.getOperatorType(operatorLexeme) != Parser::OperatorEvalType::Constant)
		throw EvaluationDefinitionError("operator " + operatorLexeme + " is not consist in constants list.");
	mConstantOperatorFunctions[operatorLexeme] = operatorDefinition;
}

template <Arithmetic Number>
Result<Number> Evaluate<Number>::evaluateExpressionTree(NodeFactory::NodePos root) const {
	std::stack<NodeFactory::NodePos> operationStack;
	std::unordered_map<NodeFactory::NodePos, Number> resultMap;

	operationStack.push(root);
	while (!operationStack.empty()) {
		const NodeFactory::NodePos currNodePos = operationStack.top();
		const NodeFactory::Node& currNode = NodeFactory::node(currNodePos);

		if (!NodeFactory::validNode(currNodePos)) continue;

		// if currNode is a leaf node.
		else if (!NodeFactory::validNode(currNode.rightPos) &&
			!NodeFactory::validNode(currNode.leftPos)) {
			if (currNode.value == ".")
				resultMap[currNodePos] = 0;

			else if (parser.isOperator(currNode.value) &&
				parser.getOperatorType(currNode.value) == Parser::OperatorEvalType::Constant) {
				Result<Number> res(evaluateConstant(currNode.value));
				EXCEPT_RETURN(res);
				resultMap[currNodePos] = res.getValue();
			}
			else
				resultMap[currNodePos] = std::stold(currNode.value);
		}

		else if (currNode.nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
			const std::vector<std::string>& parameters = currNode.utilityStorage;
			std::vector<Number> arguments;
			Parser pas(this->parser);
			Evaluate eval(pas, *this);

			NodeFactory::NodePos currArgNodePos = currNode.rightPos;
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<Number> result = evaluateExpressionTree(NodeFactory::node(currArgNodePos).leftPos);
				EXCEPT_RETURN(result);
				arguments.push_back(result.getValue());
				currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
			}

			if (arguments.size() != parameters.size())
				return ParserSyntaxError(std::format("parameters size must be equal to argument size! ({}!={})", arguments.size(), parameters.size()));

			for (size_t ind{ 0 }, len{ parameters.size() }; ind < len; ind++) {
				eval.mConstantOperatorFunctions[parameters[ind]] = [ind, &arguments]() {return arguments[ind]; };
				pas.addOperatorEvalType(parameters[ind], Parser::OperatorEvalType::Constant);
			}

			Result<Number> leftVal = eval.evaluateExpressionTree(currNode.leftPos);
			EXCEPT_RETURN(leftVal);

			resultMap[currNodePos] = leftVal.getValue();
		}

		else if (currNode.nodestate == NodeFactory::Node::NodeState::Storage) {
			std::vector<Number> arguments;

			NodeFactory::NodePos currArgNodePos = currNode.leftPos;
			while (NodeFactory::validNode(currArgNodePos)) {
				Result<Number> result = evaluateExpressionTree(NodeFactory::node(currArgNodePos).leftPos);
				EXCEPT_RETURN(result);
				arguments.push_back(result.getValue());
				currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
			}

			if (!arguments.size())
				return EvaluationFailedError("Cannot evalutate noting.");

			resultMap[currNodePos] = arguments.back();
		}

		else if (parser.getOperatorType(currNode.value) == Parser::OperatorEvalType::Infix) {
			if (!resultMap.contains(currNode.leftPos)) {
				operationStack.push(currNode.leftPos);
				continue;
			}

			if (!resultMap.contains(currNode.rightPos)) {
				operationStack.push(currNode.rightPos);
				continue;
			}

			Number leftVal{ resultMap[currNode.leftPos] };
			Number rightVal{ resultMap[currNode.rightPos] };

			Result<Number> res{ evaluateInfix(currNode.value, leftVal, rightVal) };
			EXCEPT_RETURN(res);
			resultMap[currNodePos] = res.getValue();
		}

		else if (parser.getOperatorType(currNode.value) == Parser::OperatorEvalType::Postfix) {
			if (!resultMap.contains(currNode.rightPos)) {
				operationStack.push(currNode.rightPos);
				continue;
			}

			Number rightVal{ resultMap[currNode.rightPos] };
			Result<Number> res{ evaluatePostfix(currNode.value, rightVal) };
			EXCEPT_RETURN(res);
			resultMap[currNodePos] = res.getValue();
		}

		else if (parser.getOperatorType(currNode.value) == Parser::OperatorEvalType::Prefix) {
			if (!resultMap.contains(currNode.leftPos)) {
				operationStack.push(currNode.leftPos);
				continue;
			}

			Number leftVal{ resultMap[currNode.leftPos] };
			Result<Number> res{ evaluatePrefix(currNode.value, leftVal) };
			EXCEPT_RETURN(res);
			resultMap[currNodePos] = res.getValue();
		}

		operationStack.pop();
	}

	if (!resultMap.contains(root))
		return EvaluationFailedError("failed.");

	return resultMap[root];
}

template <Arithmetic Number>
Result<Number> Evaluate<Number>::evaluateInfix(const Parser::Lexeme& opr, Number left, Number right) const {
	if (!mInfixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mInfixOperatorFunctions->at(opr)(left, right);
}

template <Arithmetic Number>
Result<Number> Evaluate<Number>::evaluatePostfix(const Parser::Lexeme& opr, Number right) const {
	if (!mPostfixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPostfixOperatorFunctions->at(opr)(right);
}

template <Arithmetic Number>
Result<Number> Evaluate<Number>::evaluatePrefix(const Parser::Lexeme& opr, Number left) const {
	if (!mPrefixOperatorFunctions->contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mPrefixOperatorFunctions->at(opr)(left);
}

template <Arithmetic Number>
Result<Number> Evaluate<Number>::evaluateConstant(const Parser::Lexeme& opr) const {
	if (!mConstantOperatorFunctions.contains(opr))
		return EvaluationDefinitionError("Definition for operator " + opr + " not found.");
	return mConstantOperatorFunctions.at(opr)();
}

#endif // EVALUATION_IMPL_H