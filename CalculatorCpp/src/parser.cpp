#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include "parser.h"
#include <sstream>

static bool isNumber(const std::string& lexeme) {
	if (lexeme.empty())
		return false;
	if (lexeme.length() == 1 && (lexeme[0] == '-' || lexeme[0] == '.'))
		return true;
	if (lexeme.length() == 2 && (lexeme[0] == '-' && lexeme[1] == '.')
		|| ((lexeme[0] == '-' || lexeme[0] == '.') && std::isdigit(lexeme[1])))
		return true;
	if (lexeme.length() >= 3 && (std::isdigit(lexeme[2]) || std::isdigit(lexeme[0])))
		return true;

	return std::isdigit(lexeme[0]);
}

bool Parser::strictedIsNumber(const std::string& lexeme) {
	if (lexeme.empty())
		return false;
	if (lexeme.length() == 1 && lexeme[0] == '.')
		return true;
	if (lexeme.length() == 2 && (lexeme[0] == '-' && lexeme[1] == '.')
		|| ((lexeme[0] == '-' || lexeme[0] == '.') && std::isdigit(lexeme[1])))
		return true;
	if (lexeme.length() >= 3 && (std::isdigit(lexeme[2]) || std::isdigit(lexeme[0])))
		return true;

	return std::isdigit(lexeme[0]);
}

static std::string_view trimLeadingZeros(const std::string& str) {
	size_t start = str.find_first_not_of('0');
	size_t sep = str.find('.');
	if (str == ".")
		return std::string_view(str);
	if (start != std::string::npos)
		return std::string_view(str.c_str() + start - (sep >= start && sep != std::string::npos ? 1 : 0), (str.length() - start));
	return std::string_view(str);
}

Parser::Node::Node(const GeneralLexeme& value) : value{ value } {}

void Parser::setBracketOperators(const std::vector<std::pair<BracketLexeme, BracketLexeme>>& bracketPairs) {
	mIsParserReady = false;
	for (const auto& [openBracket, closeBracket] : bracketPairs) {
		mBracketsOperators.openBracketsOperators[openBracket] = closeBracket;
		mBracketsOperators.closeBracketsOperators[closeBracket] = openBracket;
	}
}

void Parser::setOperatorLevels(const std::vector<std::pair<OperatorLexeme, OperatorLevel>>& operatorPairs) {
	mIsParserReady = false;
	for (const auto& [lexeme, level] : operatorPairs)
		mOperatorLevels[lexeme] = level;
}

void Parser::setOperatorEvalType(const std::vector<std::pair<OperatorLexeme, OperatorEvalType>>& operatorEvalTypePairs) {
	mIsParserReady = false;
	for (const auto& [lexeme, evalType] : operatorEvalTypePairs)
		mOperatorEvalTypes[lexeme] = evalType;
}

bool Parser::isOperator(const GeneralLexeme& lexeme) const {
	return mOperatorEvalTypes.contains(lexeme) || mOperatorLevels.contains(lexeme);
}

Parser::OperatorEvalType Parser::getOperatorType(const OperatorLexeme& oprLexeme) const {
	return mOperatorEvalTypes.at(oprLexeme);
}

std::vector<std::string> Parser::parseNumbers(const std::vector<GeneralLexeme>& lexemes) const {
	std::vector<std::string> result;
	std::string numberBuffer{ "" };
	bool foundDecimalPoint{ false };
	bool foundMinusSign{ false };

	numberBuffer.reserve(50);
	result.reserve(50);

	for (const std::string& lexeme : lexemes) {
		if ((isNumber(numberBuffer) || numberBuffer.empty()) && lexeme == "." && !foundDecimalPoint)
			foundDecimalPoint = true;

		else if (result.empty() && numberBuffer.empty() && lexeme == "-" && !foundMinusSign)
			foundMinusSign = true;

		// gg debug this code
		else if (!numberBuffer.empty() &&
			((lexeme == "." && foundDecimalPoint) ||
				(lexeme == "-" && foundMinusSign) ||
				(!((!result.empty() &&
					((mOperatorEvalTypes.contains(result.back()) &&
						(mOperatorEvalTypes.at(result.back()) == OperatorEvalType::Infix ||
							mOperatorEvalTypes.at(result.back()) == OperatorEvalType::Postfix)) ||
						mBracketsOperators.openBracketsOperators.contains(result.back()))) ||
					result.empty())) ||
				(!std::isdigit(lexeme[0]) && isNumber(numberBuffer)) ||
				(std::isdigit(lexeme[0]) && !isNumber(numberBuffer)) ||
				(!std::isdigit(lexeme[0]) && !isNumber(numberBuffer)))) {
			result.push_back(numberBuffer);
			foundDecimalPoint = false;
			foundMinusSign = false;
			numberBuffer.clear();
		}

		numberBuffer += lexeme;
	}

	if (!numberBuffer.empty())
		result.push_back(numberBuffer);

	return result;
}

void Parser::parserReady() {
	mIsParserReady = false;
	if (mBracketsOperators.openBracketsOperators.empty())
		throw ParserNotReadyError("Please setBracketsOperators.");
	if (mOperatorEvalTypes.empty())
		throw ParserNotReadyError("Please setOperatorEvalTypes.");
	if (mOperatorLevels.empty())
		throw ParserNotReadyError("Please setOperatorLevels.");
	if (mOperatorEvalTypes.size() != mOperatorLevels.size())
		throw ParserNotReadyError("Please make sure all operator set EvalType and Levels.");
	for (const auto& [key, _] : mOperatorLevels) {
		if (!mOperatorEvalTypes.contains(key))
			throw ParserNotReadyError("Operator \"" + key + "\" not found in operatorEvalTypes. Please make sure all operator set EvalType and Levels, and is the same.");
	}
	mIsParserReady = true;
}

static void processNode(Parser::Node* operatorNode, Parser::Node* operandNode1, Parser::Node* operandNode2) {
	operatorNode->left = operandNode1;
	operatorNode->right = operandNode2;
}

// @throws std::runtime_error if stack is empty
template <typename T>
static T topPopNotEmpty(std::stack<T>& stk) {
	if (stk.empty())
		throw ParserSyntaxError("heck if bracket is closed, or operator argument is valid.");
	T temp = stk.top();
	stk.pop();
	return temp;
}

Parser::Node* Parser::createOperatorTree(const std::vector<GeneralLexeme>& parsedLexemes) const {
	if (!mIsParserReady)
		throw ParserNotReadyError("Please run parserReady() first!, To make sure that parser is ready.");

	std::stack<Node*> resultStack;
	std::stack<GeneralLexeme> operatorStack;

	for (const std::string& parsedLexeme : parsedLexemes) {
		// if is a operand
		if (strictedIsNumber(parsedLexeme)) {
			// if is a argument of postfix operator
			if (!operatorStack.empty() && mOperatorEvalTypes.contains(operatorStack.top()) && mOperatorEvalTypes.at(operatorStack.top()) == OperatorEvalType::Postfix) {
				Node* operatorNode = new Node(topPopNotEmpty(operatorStack));
				Node* prefixOperandNode = new Node(parsedLexeme);
				operatorNode->right = prefixOperandNode;
				resultStack.push(operatorNode);
				continue;
			}
			resultStack.push(new Node(parsedLexeme));
		}

		// if stack is empty, if is open bracket, if top stack is open bracket
		else if (!mBracketsOperators.closeBracketsOperators.contains(parsedLexeme) &&
			!(mOperatorEvalTypes.contains(parsedLexeme) && mOperatorEvalTypes.at(parsedLexeme) == OperatorEvalType::Prefix && !resultStack.empty()) &&
			!(mOperatorEvalTypes.contains(parsedLexeme) && mOperatorEvalTypes.at(parsedLexeme) == OperatorEvalType::Postfix) &&
			(operatorStack.empty() ||
				mBracketsOperators.openBracketsOperators.contains(parsedLexeme) ||
				mBracketsOperators.openBracketsOperators.contains(operatorStack.top())))
			operatorStack.push(parsedLexeme);

		// if postfix operator, ignore
		else if (mOperatorEvalTypes.contains(parsedLexeme)
			&& mOperatorEvalTypes.at(parsedLexeme) == OperatorEvalType::Postfix) {
			operatorStack.push(parsedLexeme);
			continue;
		}

		// if is a prefix operator
		else if (mOperatorEvalTypes.contains(parsedLexeme) && mOperatorEvalTypes.at(parsedLexeme) == OperatorEvalType::Prefix && !resultStack.empty()) {
			Node* operatorNode = new Node(parsedLexeme);
			Node* prefixOperandNode = topPopNotEmpty(resultStack);
			operatorNode->left = prefixOperandNode;
			resultStack.push(operatorNode);
		}

		// if found close bracket
		else if (mBracketsOperators.closeBracketsOperators.contains(parsedLexeme)) {
			BracketLexeme openBracket{ mBracketsOperators.closeBracketsOperators.at(parsedLexeme) };
			while (!operatorStack.empty() && operatorStack.top() != openBracket) {
				Node* operatorNode = new Node(topPopNotEmpty(operatorStack));
				Node* operandNode2 = topPopNotEmpty(resultStack);
				Node* operandNode1 = topPopNotEmpty(resultStack);
				processNode(operatorNode, operandNode1, operandNode2);
				resultStack.push(operatorNode);
			}
			operatorStack.pop();

			// if current expression is argument of postfix operator
			if (!operatorStack.empty() && mOperatorEvalTypes.contains(operatorStack.top()) && mOperatorEvalTypes.at(operatorStack.top()) == OperatorEvalType::Postfix) {
				Node* operatorNode = new Node(topPopNotEmpty(operatorStack));
				Node* prefixOperandNode = topPopNotEmpty(resultStack);
				operatorNode->right = prefixOperandNode;
				resultStack.push(operatorNode);
			}
		}

		// if current operator level is higher than top stack
		else if (mOperatorLevels.at(parsedLexeme) > mOperatorLevels.at(operatorStack.top()))
			operatorStack.push(parsedLexeme);

		// if current operator level is lesser than top stack
		else if (mOperatorLevels.at(parsedLexeme) <= mOperatorLevels.at(operatorStack.top())) {
			while (!operatorStack.empty() && mOperatorLevels.contains(operatorStack.top()) && (mOperatorLevels.at(parsedLexeme) <= mOperatorLevels.at(operatorStack.top()))) {
				Node* operatorNode = new Node(topPopNotEmpty(operatorStack));
				Node* operandNode2 = topPopNotEmpty(resultStack);
				Node* operandNode1 = topPopNotEmpty(resultStack);
				processNode(operatorNode, operandNode1, operandNode2);
				resultStack.push(operatorNode);
			}
			operatorStack.push(parsedLexeme);
		}

		else
			throw ParserSyntaxError("Check if bracket is closed, or operator argument is valid.");
	}

	while (!operatorStack.empty()) {
		Node* operatorNode = new Node(topPopNotEmpty(operatorStack));
		Node* operandNode2 = topPopNotEmpty(resultStack);
		Node* operandNode1 = topPopNotEmpty(resultStack);
		processNode(operatorNode, operandNode1, operandNode2);
		resultStack.push(operatorNode);
	}

	return resultStack.top();
}

std::string Parser::printOpertatorTree(Parser::Node* tree) {
	// if tree is null
	if (tree == nullptr)
		return "";

	// if a number
	if (tree->left == nullptr && tree->right == nullptr)
		return tree->value;

	// if a operator
	std::stringstream result{};

	switch (mOperatorEvalTypes.at(tree->value))
	{
		using enum Parser::OperatorEvalType;
	case Prefix:
		result << "{" << ((tree->left != nullptr) ? printOpertatorTree(tree->left) : "null");
		result << " " << tree->value << "}";
		break;
	case Infix:
		result << "(" << ((tree->left != nullptr) ? printOpertatorTree(tree->left) : "null");
		result << " " << tree->value << " ";
		result << ((tree->right != nullptr) ? printOpertatorTree(tree->right) : "null") << ")";
		break;
	case Postfix:
		result << "[" << tree->value << " ";
		result << ((tree->right != nullptr) ? printOpertatorTree(tree->right) : "null") << "]";
		break;
	default:
		break;
	}

	return result.str();
}