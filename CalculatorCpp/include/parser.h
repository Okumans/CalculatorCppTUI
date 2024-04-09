#pragma once
#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

#include "result.h"
#include "lexer.h"

class ParserSyntaxError : public std::runtime_error {
public:
	explicit ParserSyntaxError(const std::string& msg) : std::runtime_error("ParserSyntaxError: " + msg) {}
};

class ParserNotReadyError : public std::runtime_error {
public:
	explicit ParserNotReadyError(const std::string& msg) : std::runtime_error("ParserSyntaxError: " + msg) {}
};

class Parser {
public:
	using OperatorLevel = size_t;
	using OperatorLexeme = std::string;
	using BracketLexeme = std::string;
	using NumberLexeme = std::string;
	using GeneralLexeme = std::string;

	class Node
	{
	public:
		enum class NodeState {
			None,
			LambdaFuntion,
			Storage
		}; 

		std::string value;
		Node* left{ nullptr };
		Node* right{ nullptr };
		NodeState nodestate{ NodeState::None };
		std::vector<std::string> utilityStorage;

		explicit Node(const GeneralLexeme& value);
	};

	enum class OperatorEvalType {
		Prefix,
		Infix,
		Postfix,
		Constant
	};

private:
	Brackets mBracketsOperators;
	std::unordered_map<OperatorLexeme, OperatorLevel> mOperatorLevels;
	std::unordered_map<OperatorLexeme, OperatorEvalType> mOperatorEvalTypes;
	std::unordered_map<BracketLexeme, Node::NodeState> mRawExpressionBracketEvalTypes;
	bool mIsParserReady{ false };

public:
	static bool strictedIsNumber(const std::string& lexeme, bool veryStrict = false);

	// main functions
	std::vector<GeneralLexeme> parseNumbers(const std::vector<GeneralLexeme>& lexemes) const;
	Result<std::variant<Node*, std::vector<Node*>>> createOperatorTree(const std::vector<GeneralLexeme>& parsedLexemes, bool returnVector = false) const;
	Result<Node*> createRawExpressionOperatorTree(const std::string& RawExpression) const;
	Parser::Node* createRawExpressionStorage(const std::vector<Node*>& parsedExpressions) const;
	
	// setters
	void setBracketOperators(const std::vector<std::pair<BracketLexeme, BracketLexeme>>& bracketPairs);
	void setOperatorLevels(const std::vector<std::pair<OperatorLexeme, OperatorLevel>>& operatorPairs);
	void addOperatorLevel(const OperatorLexeme& operatorLexeme, OperatorLevel operatorLevel);
	void addBracketOperator(const BracketLexeme& openBracket, const BracketLexeme& closeBracket);
	void setOperatorEvalType(const std::vector<std::pair<OperatorLexeme, OperatorEvalType>>& operatorEvalTypePairs);
	void addOperatorEvalType(const OperatorLexeme& operatorLexme, OperatorEvalType operatorEvalType);
	void setRawExpressionBracketEvalType(const std::vector<std::pair<BracketLexeme, Node::NodeState>>& rawExpressionBracketEvalTypePairs);
	void addRawExpressionBracketEvalType(const BracketLexeme& openBracketLexeme, Node::NodeState rawExpressionBracketEvalType);
	
	bool isOperator(const GeneralLexeme& lexeme) const;
	OperatorEvalType getOperatorType(const OperatorLexeme& oprLexeme) const;
	std::string printOpertatorTree(Node* tree) const;

	std::optional<ParserNotReadyError> parserReady();
	void _ignore_parserReady();
	static void freeOperatorTree(Node* tree);

};