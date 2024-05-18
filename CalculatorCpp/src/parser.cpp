#include <cctype>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <sstream>
#include <ranges>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>
#include <format>
#include <regex>
#include <algorithm>

#include "parser.h"
#include "runtimeType.h"
#include "result.h"
#include "lexer.h"
#include "runtimeType.h"
#include "runtimeTypedExprComponent.h"
#include "runtime_error.h"

#define DEBUG
#include "debug.cpp"

#define N_PARSER
#define N_EVALUATE
#include "initialization.h"

constexpr size_t STACK_CALL_LIMIT = 500;

static int randomNumber() {
	static std::mt19937 gen(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
	static std::uniform_int_distribution dis(0, 16777215);

	return dis(gen);
}

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

	return std::isdigit(lexeme[lexeme.front() == '-' || lexeme.front() == '.']);
}

bool Parser::strictedIsNumber(const std::string& lexeme, bool veryStrict) {
	if (lexeme.empty())
		return false;

	if (lexeme.length() == 1 && lexeme[0] == '.')
		return true && !veryStrict;

	if ((lexeme.length() >= 2 && (lexeme[0] == '-' && lexeme[1] == '.') && (std::isdigit(lexeme.back()) || lexeme.length() <= 2))
		|| ((lexeme[0] == '-' || lexeme[0] == '.') && std::isdigit(lexeme[1])))
		return true && !(lexeme.back() == '.' && veryStrict);

	return std::isdigit(lexeme[lexeme.front() == '-' || lexeme.front() == '.']) && (std::isdigit(lexeme.back()) || !veryStrict);
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

void Parser::setBracketOperators(const std::vector<std::pair<Lexeme, Lexeme>>& bracketPairs) {
	mIsParserReady = false;
	for (const auto& [openBracket, closeBracket] : bracketPairs) {
		mBracketsOperators.openBracketsOperators[openBracket] = closeBracket;
		mBracketsOperators.closeBracketsOperators[closeBracket] = openBracket;
	}
}

void Parser::setOperatorLevels(const std::vector<std::pair<Lexeme, OperatorLevel>>& operatorPairs) {
	mIsParserReady = false;
	for (const auto& [lexeme, level] : operatorPairs)
		mOperatorLevels[lexeme] = level;
}

void Parser::setOperatorEvalType(const std::vector<std::pair<Lexeme, OperatorEvalType>>& operatorEvalTypePairs) {
	mIsParserReady = false;
	for (const auto& [lexeme, evalType] : operatorEvalTypePairs)
		mOperatorEvalTypes[lexeme] = evalType;
}

void Parser::addBracketOperator(const Lexeme& openBracket, const Lexeme& closeBracket)
{
	mIsParserReady = false;
	mBracketsOperators.openBracketsOperators[openBracket] = closeBracket;
	mBracketsOperators.closeBracketsOperators[closeBracket] = openBracket;
}

void Parser::addOperatorLevel(const Lexeme& operatorLexeme, OperatorLevel operatorLevel)
{
	mIsParserReady = false;
	mOperatorLevels[operatorLexeme] = operatorLevel;
}

void Parser::addOperatorEvalType(const Lexeme& operatorLexme, OperatorEvalType operatorEvalType)
{
	mIsParserReady = false;
	mOperatorEvalTypes[operatorLexme] = operatorEvalType;
}

void Parser::setRawExpressionBracketEvalType(const std::vector<std::pair<Lexeme, NodeFactory::Node::NodeState>>& rawExpressionBracketEvalTypePairs)
{
	mIsParserReady = false;
	for (const auto& [lexeme, evalType] : rawExpressionBracketEvalTypePairs)
		mRawExpressionBracketEvalTypes[lexeme] = evalType;
}

void Parser::addRawExpressionBracketEvalType(const Lexeme& openBracketLexeme, NodeFactory::Node::NodeState rawExpressionBracketEvalType)
{
	mIsParserReady = false;
	mRawExpressionBracketEvalTypes[openBracketLexeme] = rawExpressionBracketEvalType;
}

bool Parser::isOperator(const Lexeme& lexeme) const {
	return mOperatorEvalTypes.contains(lexeme) || mOperatorLevels.contains(lexeme);
}

Parser::OperatorEvalType Parser::getOperatorType(const Lexeme& oprLexeme) const {
	return mOperatorEvalTypes.at(oprLexeme);
}

Parser::OperatorLevel Parser::getOperatorLevel(const Lexeme& oprLexeme) const {
	return mOperatorLevels.at(oprLexeme);
}

std::vector<std::string> Parser::parseNumbers(const std::vector<Lexeme>& lexemes) const {
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

		// gg debug this code [fucked up counter = 2]
		else if (!numberBuffer.empty() &&
			((lexeme == "." && foundDecimalPoint) ||
				(lexeme == "-" && foundMinusSign) ||
				//(!(!result.empty() &&
				//	((mOperatorEvalTypes.contains(result.back()) &&
				//		(mOperatorEvalTypes.at(result.back()) == OperatorEvalType::Infix ||
				//			mOperatorEvalTypes.at(result.back()) == OperatorEvalType::Postfix)) ||
				//		mBracketsOperators.openBracketsOperators.contains(result.back())) ||
				//	result.empty())) ||
				(std::isdigit(lexeme[0]) && strictedIsNumber(numberBuffer, true)) || // curr is a number, buffer is a "number"
				(std::isdigit(lexeme[0]) && !isNumber(numberBuffer)) || // curr is a number, buffer is not a number
				(!std::isdigit(lexeme[0]) && isNumber(numberBuffer)) || // curr is not a number, buffer is a number
				(!std::isdigit(lexeme[0]) && !isNumber(numberBuffer)))) { // curr is not a number, buffer is not a number
			result.push_back(numberBuffer);
			foundMinusSign = (lexeme == "-");
			foundDecimalPoint = (lexeme == ".");
			numberBuffer.clear();
		}

		numberBuffer += lexeme;
	}

	if (!numberBuffer.empty())
		result.push_back(numberBuffer);

	return result;
}

std::optional<RuntimeError<ParserNotReadyError>> Parser::parserReady() {
	mIsParserReady = false;
	if (mBracketsOperators.openBracketsOperators.empty())
		return RuntimeError<ParserNotReadyError>("Please setBracketsOperators.");
	if (mOperatorEvalTypes.empty())
		return RuntimeError<ParserNotReadyError>("Please setOperatorEvalTypes.");
	if (mOperatorLevels.empty())
		return RuntimeError<ParserNotReadyError>("Please setOperatorLevels.");
	if (mOperatorEvalTypes.size() != mOperatorLevels.size())
		return RuntimeError<ParserNotReadyError>("Please make sure all operator set EvalType and Levels.");
	for (const auto& [key, _] : mOperatorLevels) {
		if (!mOperatorEvalTypes.contains(key))
			return RuntimeError<ParserNotReadyError>("Operator \"" + key + "\" not found in operatorEvalTypes. Please make sure all operator set EvalType and Levels, and is the same.");
	}
	mIsParserReady = true;
	return std::nullopt;
}

void Parser::_ignore_parserReady()
{
	mIsParserReady = true;
}

static void processNode(NodeFactory::NodePos operatorNode, NodeFactory::NodePos operandNode1, NodeFactory::NodePos operandNode2) {
	NodeFactory::node(operatorNode).leftPos = operandNode1;
	NodeFactory::node(operatorNode).rightPos = operandNode2;
}

// @throws std::runtime_error if stack is empty
template <typename T>
static Result<T> topPopNotEmpty(std::stack<T>& stk) {
	if (stk.empty())
		return RuntimeError<ParserSyntaxError>("Check if bracket is closed, or operator argument is valid.");
	T temp = stk.top();
	stk.pop();
	return temp;
}

Result<std::vector<NodeFactory::NodePos>> Parser::createOperatorTree(const std::vector<Lexeme>& parsedLexemes) const {
	if (!mIsParserReady)
		return RuntimeError<ParserNotReadyError>("Please run parserReady() first!, To make sure that parser is ready.");

	std::stack<NodeFactory::NodePos> resultStack;
	std::stack<Lexeme> operatorStack;

	const auto checkOperatorEvalTypeState = [this](const Lexeme& lexeme, OperatorEvalType checkState) {
		return (mOperatorEvalTypes.contains(lexeme) && (mOperatorEvalTypes.at(lexeme) == checkState));
		};

	for (auto it = parsedLexemes.begin(); it != parsedLexemes.end(); it++) {
		const Parser::Lexeme parsedLexeme = *it;

		// if is a operand or a constant
		if ((strictedIsNumber(parsedLexeme) || checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant)) &&
			(it == parsedLexemes.begin() || (!strictedIsNumber(*std::prev(it)) && !checkOperatorEvalTypeState(*std::prev(it), OperatorEvalType::Constant)))) {
			NodeFactory::NodePos temp{ NodeFactory::create(parsedLexeme) };
			if (checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant))
				NodeFactory::node(temp).nodestate = NodeFactory::Node::NodeState::Operator;

			resultStack.push(temp);

			// if is a argument of postfix operator
			while (!operatorStack.empty() && checkOperatorEvalTypeState(operatorStack.top(), OperatorEvalType::Postfix))
			{
				auto operatorNode = NodeFactory::create(topPopNotEmpty(operatorStack).getValue()); // guarantee that operatorNodeValue will always contains a value.
				auto prefixOperandNodeValue = topPopNotEmpty(resultStack);
				EXCEPT_RETURN(prefixOperandNodeValue);

				NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;
				NodeFactory::node(operatorNode).rightPos = prefixOperandNodeValue.getValue();
				resultStack.push(operatorNode);
			}
		}

		// if stack is empty, if is open bracket, if top stack is open bracket
		else if (!strictedIsNumber(parsedLexeme) && !checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant) &&
			!mBracketsOperators.closeBracketsOperators.contains(parsedLexeme) &&
			!(checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Prefix) && !resultStack.empty()) &&
			!(checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Postfix)) &&
			(operatorStack.empty() ||
				mBracketsOperators.openBracketsOperators.contains(parsedLexeme) ||
				mBracketsOperators.openBracketsOperators.contains(operatorStack.top())))
			operatorStack.push(parsedLexeme);

		// if postfix operator, ignore
		else if (checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Postfix)) {
			operatorStack.push(parsedLexeme);
			continue;
		}

		// if is a prefix operator
		else if (checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Prefix) && !resultStack.empty()) {
			auto operatorNode = NodeFactory::create(parsedLexeme);
			auto prefixOperandNode = topPopNotEmpty(resultStack);
			EXCEPT_RETURN(prefixOperandNode);

			NodeFactory::node(operatorNode).leftPos = prefixOperandNode.getValue();
			NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;
			resultStack.push(operatorNode);
		}

		// if found close bracket
		else if (mBracketsOperators.closeBracketsOperators.contains(parsedLexeme)) {
			Lexeme openBracket{ mBracketsOperators.closeBracketsOperators.at(parsedLexeme) };

			if (mRawExpressionBracketEvalTypes.contains(openBracket)) {
				Lexeme operatorNodeValue;
				if (const auto prevIt{ *std::prev(it) }; strictedIsNumber(prevIt) || (mOperatorEvalTypes.contains(prevIt) && mOperatorEvalTypes.at(prevIt) == OperatorEvalType::Constant)) {
					auto operatorNodeRawValue = topPopNotEmpty(resultStack);
					EXCEPT_RETURN(operatorNodeRawValue);
					operatorNodeValue = NodeFactory::node(operatorNodeRawValue.getValue()).value;
				}
				else {
					auto operatorNodeRawValue = topPopNotEmpty(operatorStack);
					EXCEPT_RETURN(operatorNodeRawValue);
					operatorNodeValue = operatorNodeRawValue.getValue();

					if (operatorNodeValue == openBracket) { // empty bracket case
						operatorNodeValue = "";
						operatorStack.emplace(openBracket);
					}
				}

				auto operatorNode = (mRawExpressionBracketEvalTypes.at(openBracket) == NodeFactory::Node::NodeState::LambdaFuntion ||
					mRawExpressionBracketEvalTypes.at(openBracket) == NodeFactory::Node::NodeState::Storage) ?
					createRawExpressionOperatorTree(operatorNodeValue, mRawExpressionBracketEvalTypes.at(openBracket)) :
					NodeFactory::create(operatorNodeValue);
				EXCEPT_RETURN(operatorNode);

				resultStack.push(operatorNode.getValue());
			}

			while (!operatorStack.empty() && operatorStack.top() != openBracket) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto operandNode2 = topPopNotEmpty(resultStack);
				auto operandNode1 = topPopNotEmpty(resultStack);

				EXCEPT_RETURN(operandNode1);
				EXCEPT_RETURN(operandNode2);
				EXCEPT_RETURN(operatorNodeValue);

				auto operatorNode = NodeFactory::create(operatorNodeValue.getValue());
				NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;

				processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());

				resultStack.push(operatorNode);
			}

			if (operatorStack.empty())
				return RuntimeError<ParserSyntaxError>("bracket closed before one open.");
			operatorStack.pop(); // error here

			if (resultStack.empty())
				return std::vector<NodeFactory::NodePos>{}; // return null

			// if current expression is argument of postfix operator
			while (!operatorStack.empty() && checkOperatorEvalTypeState(operatorStack.top(), OperatorEvalType::Postfix)) {
				auto operatorNode = NodeFactory::create(topPopNotEmpty(operatorStack).getValue()); // guarantee that operatorNodeValue will always contains a value.
				auto prefixOperandNodeValue = topPopNotEmpty(resultStack);
				EXCEPT_RETURN(prefixOperandNodeValue);

				NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;
				NodeFactory::node(operatorNode).rightPos = prefixOperandNodeValue.getValue();
				resultStack.push(operatorNode);
			}
		}

		// if current operator level is higher than top stack
		else if (!checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant) && mOperatorLevels.contains(parsedLexeme) && mOperatorLevels.contains(operatorStack.top()) &&
			mOperatorLevels.at(parsedLexeme) > mOperatorLevels.at(operatorStack.top()))
			operatorStack.push(parsedLexeme);

		// if current operator level is lesser than top stack or both current lexeme and last lexeme is a number
		else if (((strictedIsNumber(parsedLexeme) || checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant)) &&
			(strictedIsNumber(*std::prev(it)) || checkOperatorEvalTypeState(*std::prev(it), OperatorEvalType::Constant))) ||
			(mOperatorLevels.contains(parsedLexeme) && mOperatorLevels.contains(operatorStack.top()) &&
				mOperatorLevels.at(parsedLexeme) <= mOperatorLevels.at(operatorStack.top()))) {
			size_t currLexemeOperatorLevels{ strictedIsNumber(parsedLexeme) ? 0 : mOperatorLevels.at(parsedLexeme) };
			while (!operatorStack.empty() && mOperatorLevels.contains(operatorStack.top()) && (currLexemeOperatorLevels <= mOperatorLevels.at(operatorStack.top()))) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto operandNode2 = topPopNotEmpty(resultStack);
				auto operandNode1 = topPopNotEmpty(resultStack);

				EXCEPT_RETURN(operandNode1);
				EXCEPT_RETURN(operandNode2);
				EXCEPT_RETURN(operatorNodeValue);

				auto operatorNode = NodeFactory::create(operatorNodeValue.getValue());
				processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());
				NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;

				resultStack.push(operatorNode);
			}

			if (strictedIsNumber(parsedLexeme))
				resultStack.push(NodeFactory::create(parsedLexeme));
			else if (checkOperatorEvalTypeState(parsedLexeme, OperatorEvalType::Constant)) {
				NodeFactory::NodePos temp{ NodeFactory::create(parsedLexeme) };
				NodeFactory::node(temp).nodestate = NodeFactory::Node::NodeState::Operator;
				resultStack.push(temp);
			}
			else
				operatorStack.push(parsedLexeme);
		}

		else
			return RuntimeError<ParserSyntaxError>("Check if bracket is closed, or operator argument is valid.");
	}

	while (!operatorStack.empty()) {
		auto operatorNodeValue = topPopNotEmpty(operatorStack);
		auto operandNode2 = topPopNotEmpty(resultStack);
		auto operandNode1 = topPopNotEmpty(resultStack);

		EXCEPT_RETURN(operandNode1);
		EXCEPT_RETURN(operandNode2);
		EXCEPT_RETURN(operatorNodeValue);

		auto operatorNode = NodeFactory::create(operatorNodeValue.getValue());
		NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Operator;

		processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());
		resultStack.push(operatorNode);
	}

	if (resultStack.empty())
		return std::vector<NodeFactory::NodePos>{}; // return null

	std::vector<NodeFactory::NodePos> tmp(resultStack.size(), NodeFactory::NodePosNull);
	for (size_t i{ resultStack.size() }; i > 0; i--) {
		tmp[i - 1] = resultStack.top();
		resultStack.pop();
	}

	return tmp;
}

NodeFactory::NodePos Parser::createRawExpressionStorage(const std::vector<NodeFactory::NodePos>& parsedExpressions) const {
	NodeFactory::NodePos root = NodeFactory::create();
	NodeFactory::NodePos tail = root;

	NodeFactory::node(tail).leftPos = parsedExpressions.front();

	for (size_t i{ 1 }; i < parsedExpressions.size(); i++)
	{
		NodeFactory::NodePos curr = NodeFactory::create();
		NodeFactory::node(curr).leftPos = parsedExpressions[i];
		NodeFactory::node(tail).rightPos = curr;
		tail = curr;
	}
	return root;
}

bool Parser::checkIfValidParameterName(const std::string& parameter) const {
	if (strictedIsNumber(parameter, true))
		return false;
	if (mOperatorEvalTypes.contains(parameter))
		return false;
	if (mBracketsOperators.closeBracketsOperators.contains(parameter) ||
		mBracketsOperators.openBracketsOperators.contains(parameter))
		return false;
	if (mRawExpressionBracketEvalTypes.contains(parameter))
		return false;
	return true;
}

std::optional<std::runtime_error> Parser::getLambdaType(std::vector<std::pair<std::string, RuntimeType>>& parametersWithTypes, std::string parameterExpression) const {
	std::regex pattern("(\\w+)(?::(\\w+(?:\\[\\w+(?:,\\w+)?\\])?))?");
	std::smatch matches;

	while (std::regex_search(parameterExpression, matches, pattern)) {
		if (matches[2].str().empty()) {
			if (!checkIfValidParameterName(matches[1].str()))
				return RuntimeError<ParserSyntaxError>(
					std::format(
						"The parameter name \"{}\" is not valid. Parameter names cannot be operators, numbers, or brackets.",
						matches[1].str()
					)
				);

			parametersWithTypes.emplace_back(matches[1], RuntimeBaseType::Number);
		}

		else {
			Result<RuntimeType, std::runtime_error> parsedRuntimeTypedResult{ RuntimeCompoundType::ParseString(matches[2]) };

			if (parsedRuntimeTypedResult.isError())
				return RuntimeError<ParserSyntaxError>(
					parsedRuntimeTypedResult.getException(),
					std::format(
						"When attempting to parse the type of a lambda function parameter \"{}\".",
						matches[3].str()
					),
					"Parser::createRawExpressionOperatorTree");

			parametersWithTypes.emplace_back(matches[1], parsedRuntimeTypedResult.moveValue());
		}
		parameterExpression = matches.suffix().str();
	}

	return std::nullopt;
}

Result<NodeFactory::NodePos> Parser::createRawExpressionOperatorTree(const std::string& RawExpression, NodeFactory::Node::NodeState RawExpressionType) const
{
	std::string_view rawVariablesExpression;
	std::string_view rawOperationTree(RawExpression);

	bool foundParameterSlot{ false };
	Parser pas(*this); // very expensive

	std::vector<std::string> rawExpressionLexemes = initializeStaticLexer(std::vector<std::string>{ ",", ";", "Number", "Storage", "Lambda" })(RawExpression);
	std::vector<std::pair<std::string, RuntimeType>> variableLexemesWithTypes;

	auto variableSpilterIndexExist{ std::ranges::find(rawExpressionLexemes, ";") };

	if (auto variableSpilterIndex{ std::ranges::find(RawExpression, ';') };
		variableSpilterIndexExist != rawExpressionLexemes.end() && variableSpilterIndex != RawExpression.end()) {
		rawVariablesExpression = std::string_view(RawExpression.begin(), variableSpilterIndex);
		rawOperationTree = std::string_view(++variableSpilterIndex, RawExpression.end());

		foundParameterSlot = true;
		if (std::optional<std::runtime_error> parameterParsingError{ getLambdaType(variableLexemesWithTypes, std::string(rawVariablesExpression)) }; parameterParsingError.has_value())
			return parameterParsingError.value();

		for (const auto& [variableLexeme, _] : variableLexemesWithTypes) {
			pas.addOperatorEvalType(variableLexeme, Parser::OperatorEvalType::Constant);
			pas.addOperatorLevel(variableLexeme, 9);
			pas.mTempConstant.emplace(variableLexeme);
		}
	}

	auto operationTree = initializeStaticLexer(pas.mTempConstant)(std::string(rawOperationTree));
	auto parsedNumberOperationTree = parseNumbers(operationTree);

	pas._ignore_parserReady();
	auto fullyParsedOperationTree = pas.createOperatorTree(parsedNumberOperationTree);
	EXCEPT_RETURN(fullyParsedOperationTree);

	if (RawExpressionType == NodeFactory::Node::NodeState::LambdaFuntion) {
		NodeFactory::NodePos operatorNode = createRawExpressionStorage(fullyParsedOperationTree.getValue());
		NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::LambdaFuntion;
		NodeFactory::node(operatorNode).utilityStorage = variableLexemesWithTypes;
		NodeFactory::node(operatorNode).value = std::format("lambda-{:x}", randomNumber());

		return operatorNode;
	}

	else if (!foundParameterSlot && RawExpressionType == NodeFactory::Node::NodeState::Storage) {
		NodeFactory::NodePos operatorNode;

		if (RawExpression == "")
			operatorNode = NodeFactory::create();
		else
			operatorNode = createRawExpressionStorage(fullyParsedOperationTree.getValue());

		NodeFactory::node(operatorNode).value = std::format("storage-{:x}", randomNumber());
		NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::Storage;

		return operatorNode;
	}

	return RuntimeError<ParserSyntaxError>(
		std::format(
			R"(The "Storage" class cannot contain a parameter slot named "{}" as it is not valid.)",
			RawExpression
		),
		"Parser::createRawExpressionOperatorTree"
	);
}

static std::string _printOpertatorTree(NodeFactory::NodePos tree, const std::unordered_map<Parser::Lexeme, Parser::OperatorEvalType>& mOperatorEvalTypes, size_t _level) {
	const auto& treeNode = NodeFactory::node(tree); // guarantee no modification, if isn't treeNode can be dangling reference.

	// if tree is null
	if (!NodeFactory::validNode(tree))
		return "";

	if (_level > STACK_CALL_LIMIT)
		return ColorText<Color::Cyan>("...");

	// if a number
	if (!NodeFactory::validNode(treeNode.leftPos) && !NodeFactory::validNode(treeNode.rightPos)) {
		if (treeNode.nodestate == NodeFactory::Node::NodeState::Operator)
			return ColorText<Color::Bright_Magenta>(treeNode.value);
		return ColorText<Color::Bright_Blue>(treeNode.value);
	}

	std::stringstream result{};

	// if a lambda function
	if (treeNode.nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
		const std::vector <std::pair<std::string, RuntimeType>>& parameters = treeNode.utilityStorage;
		std::vector<std::string> arguments;
		std::vector<std::string> expressions;

		NodeFactory::NodePos currArgNodePos = tree;
		while (NodeFactory::validNode(currArgNodePos)) {
			expressions.push_back(_printOpertatorTree(NodeFactory::node(currArgNodePos).leftPos, mOperatorEvalTypes, _level + 1));
			currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
		}

		if (_level) {
			result << ColorText<Color::Yellow>("<");
			for (size_t i{ 0 }, par{ parameters.size() }, arg{ arguments.size() }, len{ std::max(par, arg) }; i < len; i++)
				result << ColorText<Color::Bright_Magenta>((i < par) ? parameters[i].first : "null") << ColorText<Color::Yellow>(":") << (ColorText<Color::Bright_Yellow>(RuntimeTypeToString((i < par) ? parameters[i].second : RuntimeBaseType::_Storage))) << ((i != len - 1) ? ColorText<Color::Cyan>(", ") : "");
			result << ColorText<Color::Yellow>(">");
		}

		result << ColorText<Color::Yellow>("{");
		for (size_t ind{ 0 }; ind < expressions.size(); ind++)
			result << expressions[ind] << ((ind != expressions.size() - 1) ? ColorText<Color::Cyan>(", ") : "");

		result << ColorText<Color::Yellow>("}");
		return result.str();
	}

	// if a stoarge
	if (treeNode.nodestate == NodeFactory::Node::NodeState::Storage) {
		result << ColorText<Color::Yellow>("[");
		NodeFactory::NodePos curr = tree;
		while (NodeFactory::validNode(curr)) {
			result << _printOpertatorTree(NodeFactory::node(curr).leftPos, mOperatorEvalTypes, _level + 1) << ", ";
			curr = NodeFactory::node(curr).rightPos;
		}
		result.seekp(-2, std::ios_base::end);
		result << ColorText<Color::Yellow>("]");
		return result.str();
	}

	// if a operator
	switch (mOperatorEvalTypes.at(treeNode.value))
	{
		using enum Parser::OperatorEvalType;
	case Prefix:
		result << (NodeFactory::validNode(treeNode.leftPos) ? _printOpertatorTree(treeNode.leftPos, mOperatorEvalTypes, _level + 1) : "null")
			<< " " << ColorText<Color::Cyan>(treeNode.value);
		break;
	case Infix:
		result << (NodeFactory::validNode(treeNode.leftPos) ? _printOpertatorTree(treeNode.leftPos, mOperatorEvalTypes, _level + 1) : "null")
			<< " " << ColorText<Color::Cyan>(treeNode.value) << " "
			<< (NodeFactory::validNode(treeNode.rightPos) ? _printOpertatorTree(treeNode.rightPos, mOperatorEvalTypes, _level + 1) : "null");
		break;
	case Postfix:
		result << ColorText<Color::Cyan>(treeNode.value) << " "
			<< (NodeFactory::validNode(treeNode.rightPos) ? _printOpertatorTree(treeNode.rightPos, mOperatorEvalTypes, _level + 1) : "null");
		break;
	default:
		break;
	}

	return result.str();
}

std::string Parser::printOpertatorTree(std::vector<NodeFactory::NodePos> trees, const std::unordered_map<Parser::Lexeme, Lambda>& EvaluatorLambdaFunctions) const {
	std::vector<NodeFactory::NodePos> evalutationResults;

	std::ranges::reverse(trees);

	while (!trees.empty()) {
		NodeFactory::NodePos currNode = trees.back(); trees.pop_back();

		if (NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
			NodeFactory::NodePos operatorNode{ NodeFactory::create() };
			NodeFactory::node(operatorNode).nodestate = NodeFactory::Node::NodeState::LambdaFuntion;
			NodeFactory::node(operatorNode).leftPos = currNode;

			evalutationResults.emplace_back(operatorNode);
			continue;
		}

		if (NodeFactory::node(currNode).nodestate == NodeFactory::Node::NodeState::Storage) {
			Result<Storage, std::runtime_error> storageResult{
				Storage::fromExpressionNode(currNode, EvaluatorLambdaFunctions) };

			if (storageResult.isError())
				return RuntimeError<StorageEvaluationError>(
					storageResult.getException(),
					std::format(
						"When attempting to convert a NodeExpression into a storage for evaluation. (nodeExpression value = {})",
						NodeFactory::validNode(currNode) ? NodeFactory::node(currNode).value : "Null"
					),
					"Lambda::_NodeExpressionsEvaluator"
				).what();

			if (evalutationResults.size() && NodeFactory::node(evalutationResults.back()).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
				if (NodeFactory::validNode(NodeFactory::node(evalutationResults.back()).rightPos) &&
					NodeFactory::node(evalutationResults.back()).rightNode().nodestate == NodeFactory::Node::NodeState::Storage) {
					evalutationResults.emplace_back(currNode);
					continue;
				}

				NodeFactory::NodePos operatorNode{ evalutationResults.back() };
				NodeFactory::node(operatorNode).rightPos = storageResult.getValue().getNodeExpression();
				evalutationResults.pop_back();
				evalutationResults.emplace_back(operatorNode);
				continue;
			}

			evalutationResults.emplace_back(storageResult.getValue().generateExpressionTree());
			continue;
		}

		evalutationResults.emplace_back(currNode);
	}

	std::stringstream result;
	for (size_t ind{ 0 }; ind < evalutationResults.size(); ind++) {
		NodeFactory::NodePos root{ evalutationResults[ind] };

		if (NodeFactory::node(root).nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
			const std::vector<std::pair<std::string, RuntimeType>>& parameters{ NodeFactory::node(root).leftNode().utilityStorage };
			std::vector<NodeFactory::NodePos> arguments;

			NodeFactory::NodePos currNodePos = NodeFactory::node(root).rightPos;
			while (NodeFactory::validNode(currNodePos)) {
				arguments.emplace_back(NodeFactory::node(currNodePos).leftPos);
				currNodePos = NodeFactory::node(currNodePos).rightPos;
			}

			result << ColorText<Color::Yellow>("<");
			for (size_t i{ 0 }, par{ parameters.size() }, arg{ arguments.size() }, len{ std::max(par, arg) }; i < len; i++) {
				result << ColorText<Color::Bright_Magenta>((i < par) ? parameters[i].first : "null") << ColorText<Color::Yellow>(":") << (ColorText<Color::Bright_Yellow>(RuntimeTypeToString((i < par) ? parameters[i].second : RuntimeBaseType::_Storage))) << (arg ? (ColorText<Color::Yellow>("(") + ((i < arg) ? _printOpertatorTree(arguments[i], mOperatorEvalTypes, 0) : "null") + ColorText<Color::Yellow>(")")) : "") << ((i != len - 1) ? ColorText<Color::Cyan>(", ") : "");
			}
			result << ColorText<Color::Yellow>(">");
			result << _printOpertatorTree(NodeFactory::node(root).leftPos, mOperatorEvalTypes, 0) << ((ind != evalutationResults.size() - 1) ? ColorText<Color::Cyan>(", ") : "");
			continue;
		}
		result << _printOpertatorTree(root, mOperatorEvalTypes, 0) << ((ind != evalutationResults.size() - 1) ? ColorText<Color::Cyan>(", ") : "");
	}
	return result.str();
}