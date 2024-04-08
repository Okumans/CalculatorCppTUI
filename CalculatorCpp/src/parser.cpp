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

#include "parser.h"
#include "result.h"
#include "lexer.h"

#define DEBUG
#include "debug.cpp"

#define N_PARSER
#define N_EVALUATE
#include "initialization.h"

static std::string strip(std::string in)
{
	in.erase(std::remove_if(in.begin(), in.end(), [](std::string::value_type ch)
		{ return !isalpha(ch); }
	), in.end());
	return in;
}

static auto splitString(std::string_view in, char sep) {
	std::vector<std::string_view> r;
	r.reserve(std::count(in.begin(), in.end(), sep) + 1); // optional
	for (auto p = in.begin();; ++p) {
		auto q = p;
		p = std::find(p, in.end(), sep);
		r.emplace_back(q, p);
		if (p == in.end())
			return r;
	}
}

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

	return std::isdigit(lexeme[0]);
}

bool Parser::strictedIsNumber(const std::string& lexeme, bool veryStrict) {
	if (lexeme.empty())
		return false;
	if (lexeme.length() == 1 && lexeme[0] == '.')
		return true && !veryStrict;
	if (lexeme.length() == 2 && (lexeme[0] == '-' && lexeme[1] == '.')
		|| ((lexeme[0] == '-' || lexeme[0] == '.') && std::isdigit(lexeme[1])))
		return true && !veryStrict;
	if (lexeme.length() >= 3 && (std::isdigit(lexeme[2]) || std::isdigit(lexeme[0])))
		return true;

	return std::isdigit(lexeme[0]) && (std::isdigit(lexeme.back()) || !veryStrict);
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

Parser::Node::Node(const GeneralLexeme& value) : value{ value } {/*std::cout << "(create " << value << ") "; */ }

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

void Parser::addBracketOperator(const BracketLexeme& openBracket, const BracketLexeme& closeBracket)
{
	mIsParserReady = false;
	mBracketsOperators.openBracketsOperators[openBracket] = closeBracket;
	mBracketsOperators.closeBracketsOperators[closeBracket] = openBracket;
}

void Parser::addOperatorLevel(const OperatorLexeme& operatorLexeme, OperatorLevel operatorLevel)
{
	mIsParserReady = false;
	mOperatorLevels[operatorLexeme] = operatorLevel;
}

void Parser::addOperatorEvalType(const OperatorLexeme& operatorLexme, OperatorEvalType operatorEvalType)
{
	mIsParserReady = false;
	mOperatorEvalTypes[operatorLexme] = operatorEvalType;
}

void Parser::setRawExpressionBracketEvalType(const std::vector<std::pair<BracketLexeme, Node::NodeState>>& rawExpressionBracketEvalTypePairs)
{
	mIsParserReady = false;
	for (const auto& [lexeme, evalType] : rawExpressionBracketEvalTypePairs)
		mRawExpressionBracketEvalTypes[lexeme] = evalType;
}

void Parser::addRawExpressionBracketEvalType(const BracketLexeme& openBracketLexeme, Node::NodeState rawExpressionBracketEvalType)
{
	mIsParserReady = false;
	mRawExpressionBracketEvalTypes[openBracketLexeme] = rawExpressionBracketEvalType;
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
				(std::isdigit(lexeme[0]) && strictedIsNumber(numberBuffer, true)) ||
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

std::optional<ParserNotReadyError> Parser::parserReady() {
	mIsParserReady = false;
	if (mBracketsOperators.openBracketsOperators.empty())
		return ParserNotReadyError("Please setBracketsOperators.");
	if (mOperatorEvalTypes.empty())
		return ParserNotReadyError("Please setOperatorEvalTypes.");
	if (mOperatorLevels.empty())
		return ParserNotReadyError("Please setOperatorLevels.");
	if (mOperatorEvalTypes.size() != mOperatorLevels.size())
		return ParserNotReadyError("Please make sure all operator set EvalType and Levels.");
	for (const auto& [key, _] : mOperatorLevels) {
		if (!mOperatorEvalTypes.contains(key))
			return ParserNotReadyError("Operator \"" + key + "\" not found in operatorEvalTypes. Please make sure all operator set EvalType and Levels, and is the same.");
	}
	mIsParserReady = true;
	return std::nullopt;
}

void Parser::_ignore_parserReady()
{
	mIsParserReady = true;
}

static void processNode(Parser::Node* operatorNode, Parser::Node* operandNode1, Parser::Node* operandNode2) {
	operatorNode->left = operandNode1;
	operatorNode->right = operandNode2;
}

// @throws std::runtime_error if stack is empty
template <typename T>
static Result<T> topPopNotEmpty(std::stack<T>& stk) {
	if (stk.empty())
		return ParserSyntaxError("Check if bracket is closed, or operator argument is valid.");
	T temp = stk.top();
	stk.pop();
	return temp;
}

Result<std::variant<Parser::Node*, std::vector<Parser::Node*>>> Parser::createOperatorTree(const std::vector<GeneralLexeme>& parsedLexemes, bool returnVector) const {
	if (!mIsParserReady)
		return ParserNotReadyError("Please run parserReady() first!, To make sure that parser is ready.");

	std::stack<Node*> resultStack;
	std::stack<GeneralLexeme> operatorStack;

	for (auto it = parsedLexemes.begin(); it != parsedLexemes.end(); it++) {
		const Parser::GeneralLexeme parsedLexeme = *it;

		// if is a operand or a constant
		if (strictedIsNumber(parsedLexeme) || (mOperatorEvalTypes.contains(parsedLexeme) && mOperatorEvalTypes.at(parsedLexeme) == OperatorEvalType::Constant)) {
			// if is a argument of postfix operator
			if (!operatorStack.empty() && mOperatorEvalTypes.contains(operatorStack.top()) && mOperatorEvalTypes.at(operatorStack.top()) == OperatorEvalType::Postfix) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto prefixOperandNode = new Node(parsedLexeme);

				EXCEPT_RETURN(operatorNodeValue,
					delete prefixOperandNode
				);

				auto operatorNode = new Node(operatorNodeValue.getValue());
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
			auto operatorNode = new Node(parsedLexeme);
			auto prefixOperandNode = topPopNotEmpty(resultStack);
			EXCEPT_RETURN(prefixOperandNode, delete operatorNode);

			operatorNode->left = prefixOperandNode.getValue();
			resultStack.push(operatorNode);
		}

		// if found close bracket
		else if (mBracketsOperators.closeBracketsOperators.contains(parsedLexeme)) {
			BracketLexeme openBracket{ mBracketsOperators.closeBracketsOperators.at(parsedLexeme) };

			if (mRawExpressionBracketEvalTypes.contains(openBracket)) {
				GeneralLexeme operatorNodeValue;
				if (const auto prevIt{ *std::prev(it) }; strictedIsNumber(prevIt) || (mOperatorEvalTypes.contains(prevIt) && mOperatorEvalTypes.at(prevIt) == OperatorEvalType::Constant))
				{
					auto operatorNodeRawValue = topPopNotEmpty(resultStack);
					EXCEPT_RETURN(operatorNodeRawValue);
					operatorNodeValue = operatorNodeRawValue.getValue()->value;
				}
				else
				{
					auto operatorNodeRawValue = topPopNotEmpty(operatorStack);
					EXCEPT_RETURN(operatorNodeRawValue);
					operatorNodeValue = operatorNodeRawValue.getValue();
				}

				auto operatorNode = (mRawExpressionBracketEvalTypes.at(openBracket) == Node::NodeState::LambdaFuntion) ?
					createRawExpressionOperatorTree(operatorNodeValue) :
					new Node(operatorNodeValue);
				EXCEPT_RETURN(operatorNode);

				resultStack.push(operatorNode.getValue());
			}

			while (!operatorStack.empty() && operatorStack.top() != openBracket) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto operandNode2 = topPopNotEmpty(resultStack);
				auto operandNode1 = topPopNotEmpty(resultStack);

				EXCEPT_RETURN(operandNode1,
					EXCEPT_RETURN_N(operandNode2,
						freeOperatorTree(operandNode2.getValue())));
				EXCEPT_RETURN(operandNode2,
					freeOperatorTree(operandNode1.getValue()));
				EXCEPT_RETURN(operatorNodeValue);

				auto operatorNode = new Node(operatorNodeValue.getValue());

				processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());

				resultStack.push(operatorNode);
			}

			if (operatorStack.empty())
				return ParserSyntaxError("bracket closed before one open.");
			operatorStack.pop(); // error here

			// if current expression is argument of lambda function
			if (resultStack.top()->nodestate == Node::NodeState::Storage) {
				auto storageNode = topPopNotEmpty(resultStack);
				auto lambdaNode = topPopNotEmpty(resultStack);

				if (!lambdaNode.isError() && lambdaNode.getValue()->nodestate == Node::NodeState::LambdaFuntion) {
					lambdaNode.getValue()->right = storageNode.getValue()->left;
					resultStack.push(lambdaNode.getValue());
				}
				else {
					resultStack.push(storageNode.getValue());
					if (!lambdaNode.isError())
						resultStack.push(lambdaNode.getValue());
				}
			}

			// if current expression is argument of postfix operator
			if (!operatorStack.empty() && mOperatorEvalTypes.contains(operatorStack.top()) && mOperatorEvalTypes.at(operatorStack.top()) == OperatorEvalType::Postfix) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto prefixOperandNode = topPopNotEmpty(resultStack);

				EXCEPT_RETURN(prefixOperandNode);
				EXCEPT_RETURN(operatorNodeValue,
					freeOperatorTree(prefixOperandNode.getValue()));

				auto operatorNode = new Node(operatorNodeValue.getValue());
				operatorNode->right = prefixOperandNode.getValue();
				resultStack.push(operatorNode);
			}
		}

		// if current operator level is higher than top stack
		else if (mOperatorLevels.contains(parsedLexeme) && mOperatorLevels.contains(operatorStack.top()) &&
			mOperatorLevels.at(parsedLexeme) > mOperatorLevels.at(operatorStack.top()))
			operatorStack.push(parsedLexeme);

		// if current operator level is lesser than top stack
		else if (mOperatorLevels.contains(parsedLexeme) && mOperatorLevels.contains(operatorStack.top()) &&
			mOperatorLevels.at(parsedLexeme) <= mOperatorLevels.at(operatorStack.top())) {
			while (!operatorStack.empty() && mOperatorLevels.contains(operatorStack.top()) && (mOperatorLevels.at(parsedLexeme) <= mOperatorLevels.at(operatorStack.top()))) {
				auto operatorNodeValue = topPopNotEmpty(operatorStack);
				auto operandNode2 = topPopNotEmpty(resultStack);
				auto operandNode1 = topPopNotEmpty(resultStack);

				EXCEPT_RETURN(operandNode1,
					EXCEPT_RETURN_N(operandNode2,
						freeOperatorTree(operandNode2.getValue())));
				EXCEPT_RETURN(operandNode2,
					freeOperatorTree(operandNode1.getValue()));
				EXCEPT_RETURN(operatorNodeValue);

				auto operatorNode = new Node(operatorNodeValue.getValue());
				processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());

				resultStack.push(operatorNode);
			}
			operatorStack.push(parsedLexeme);
		}

		else
			return ParserSyntaxError("Check if bracket is closed, or operator argument is valid.");
	}

	while (!operatorStack.empty()) {
		auto operatorNodeValue = topPopNotEmpty(operatorStack);
		auto operandNode2 = topPopNotEmpty(resultStack);
		auto operandNode1 = topPopNotEmpty(resultStack);

		EXCEPT_RETURN(operandNode1,
			EXCEPT_RETURN_N(operandNode2,
				freeOperatorTree(operandNode2.getValue())));
		EXCEPT_RETURN(operandNode2,
			freeOperatorTree(operandNode1.getValue()));
		EXCEPT_RETURN(operatorNodeValue);

		auto operatorNode = new Node(operatorNodeValue.getValue());

		processNode(operatorNode, operandNode1.getValue(), operandNode2.getValue());
		resultStack.push(operatorNode);
	}

	if (resultStack.empty())
		return ParserSyntaxError("Noting to parsed, value required!");

	if (returnVector)
	{
		std::vector<Node*> tmp;
		while (!resultStack.empty()) {
			tmp.push_back(resultStack.top());
			resultStack.pop();
		}
		return std::variant<Node*, std::vector<Node*>>(tmp);
	}

	return std::variant<Node*, std::vector<Node*>>(resultStack.top());
}

Parser::Node* Parser::createRawExpressionStorage(const std::vector<Node*>& parsedExpressions) const {
	Node* root = new Node("");
	Node* tail = root;
	tail->left = parsedExpressions.front();

	for (size_t i{ parsedExpressions.size() - 1 }; i > 0; i--)
	{
		Node* curr = new Node("");
		curr->left = parsedExpressions[i];
		tail->right = curr;
		tail = curr;
	}

	return root;
}

Result<Parser::Node*> Parser::createRawExpressionOperatorTree(const std::string& RawExpression) const
{
	std::string_view rawVariablesExpression;
	std::string_view rawOperationTree(RawExpression);
	std::vector<std::string> variableLexemes;
	bool isLambdaFunction{ false };

	Lexer lex;
	initializeLexer(lex);

	Parser pas(*this);

	if (auto variableSpilterIndex{ std::ranges::find(RawExpression, ';') }; variableSpilterIndex != RawExpression.end()) {
		rawVariablesExpression = std::string_view(RawExpression.begin(), variableSpilterIndex);
		rawOperationTree = std::string_view(++variableSpilterIndex, RawExpression.end());

		for (const auto splited : splitString(rawVariablesExpression, ','))
		{
			variableLexemes.emplace_back(splited);
			lex._addKeyword_not_reinitializeKeyWordTree(std::string(splited));
			pas.addOperatorEvalType(std::string(splited), OperatorEvalType::Constant);
			pas.addOperatorLevel(std::string(splited), 9);
		}

		isLambdaFunction = true;
	}

	lex._reinitializeKeyWordTree();

	auto operationTree = lex.lexing(rawOperationTree.data());
	auto parsedNumberOperationTree = parseNumbers(operationTree);

	std::cout << parsedNumberOperationTree << "\n";

	pas._ignore_parserReady();
	if (isLambdaFunction) {
		auto fullyParsedOperationTree = pas.createOperatorTree(parsedNumberOperationTree);

		if (fullyParsedOperationTree.isError())
			return fullyParsedOperationTree.getException();

		auto fullyParsedOperationTreeValue{ fullyParsedOperationTree.getValue() };

		auto operatorNode = new Node(std::format("lambda-{:x}", randomNumber()));
		operatorNode->nodestate = Node::NodeState::LambdaFuntion;
		operatorNode->left = std::get<Node*>(fullyParsedOperationTreeValue);
		operatorNode->utilityStorage = variableLexemes;

		return operatorNode;
	}

	else {
		auto fullyParsedOperationTree = pas.createOperatorTree(parsedNumberOperationTree, true);

		if (fullyParsedOperationTree.isError())
			return fullyParsedOperationTree.getException();

		auto fullyParsedOperationTreeValue{ fullyParsedOperationTree.getValue() };

		auto operatorNode = new Node(std::format("storage-{:x}", randomNumber()));
		operatorNode->nodestate = Node::NodeState::Storage;
		operatorNode->left = createRawExpressionStorage(std::get<std::vector<Node*>>(fullyParsedOperationTreeValue));

		return operatorNode;
	}
}

std::string Parser::printOpertatorTree(Parser::Node* tree) const {
	// if tree is null
	if (tree == nullptr)
		return "";

	// if a number
	if (tree->left == nullptr && tree->right == nullptr)
		return tree->value;

	std::stringstream result{};

	// if a lambda function
	if (tree->nodestate == Node::NodeState::LambdaFuntion) {
		result << "<";
		for (const std::string& str : tree->utilityStorage)
			result << str << ", ";
		tree->utilityStorage.size() && result.seekp(-2, std::ios_base::end);
		result << ">" << tree->value << "{" << ((tree->left != nullptr) ? printOpertatorTree(tree->left) : "null") << "}";
		return result.str();
	}

	// if a stoarge
	if (tree->nodestate == Node::NodeState::Storage) {
		result << tree->value << "[";

		if (tree->left != nullptr) {
			Node* curr = tree->left;
			while (curr != nullptr) {
				result << "(" <<printOpertatorTree(curr->left) << "), ";
				curr = curr->right;
			}
			result.seekp(-2, std::ios_base::end);
		}

		result << "]";
		return result.str();
	}

	// if a operator
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

void Parser::freeOperatorTree(Parser::Node* tree) {
	if (tree == nullptr)
		return;

	if (tree->left != nullptr) {
		freeOperatorTree(tree->left);
		tree->left = nullptr;
	}

	if (tree->right != nullptr) {
		freeOperatorTree(tree->right);
		tree->right = nullptr;
	}

	// std::cout << "(free " << tree->value << ") ";
	delete tree;
}