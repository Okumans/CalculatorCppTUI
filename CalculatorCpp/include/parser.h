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
#include "nodeFactory.h"

class ParserNotReadyError : public std::runtime_error {
public:
	explicit ParserNotReadyError(const std::string& msg) : std::runtime_error("ParserSyntaxError: " + msg) {}
};


// Custom exception class for runtime type errors
class ParserSyntaxError: public std::runtime_error {
public:
	// Constructor with a single message
	explicit ParserSyntaxError(const std::string& message)
		: std::runtime_error("ParserSyntaxError: " + message) {}

	// Constructor with message and origin information
	explicit ParserSyntaxError(const std::string& message, const std::string from)
		: std::runtime_error("ParserSyntaxError: " + message + " (from: " + from + ")") {}

	// Constructor with chained error, message, and origin information
	explicit ParserSyntaxError(const std::runtime_error& baseError, const std::string& message, const std::string from)
		: std::runtime_error("ParserSyntaxError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

class Parser {
public:
	using OperatorLevel = size_t;
	using Lexeme = std::string;

	enum class OperatorEvalType : int8_t {
		Prefix,
		Infix,
		Postfix,
		Constant
	};

private:
	Brackets mBracketsOperators;
	std::unordered_map<Lexeme, OperatorLevel> mOperatorLevels;
	std::unordered_map<Lexeme, OperatorEvalType> mOperatorEvalTypes;
	std::unordered_map<Lexeme, NodeFactory::Node::NodeState> mRawExpressionBracketEvalTypes;
	bool mIsParserReady{ false };

public:
	static bool strictedIsNumber(const std::string& lexeme, bool veryStrict = false);

	// main functions
	std::vector<Lexeme> parseNumbers(const std::vector<Lexeme>& lexemes) const;
	Result<std::variant<NodeFactory::NodePos, std::vector<NodeFactory::NodePos>>> createOperatorTree(const std::vector<Lexeme>& parsedLexemes, bool returnVector = false) const;
	Result<NodeFactory::NodePos> createRawExpressionOperatorTree(const std::string& RawExpression) const;
	NodeFactory::NodePos createRawExpressionStorage(const std::vector<NodeFactory::NodePos>& parsedExpressions) const;
	
	// setters
	void setBracketOperators(const std::vector<std::pair<Lexeme, Lexeme>>& bracketPairs);
	void setOperatorLevels(const std::vector<std::pair<Lexeme, OperatorLevel>>& operatorPairs);
	void addOperatorLevel(const Lexeme& operatorLexeme, OperatorLevel operatorLevel);
	void addBracketOperator(const Lexeme& openBracket, const Lexeme& closeBracket);
	void setOperatorEvalType(const std::vector<std::pair<Lexeme, OperatorEvalType>>& operatorEvalTypePairs);
	void addOperatorEvalType(const Lexeme& operatorLexme, OperatorEvalType operatorEvalType);
	void setRawExpressionBracketEvalType(const std::vector<std::pair<Lexeme, NodeFactory::Node::NodeState>>& rawExpressionBracketEvalTypePairs);
	void addRawExpressionBracketEvalType(const Lexeme& openBracketLexeme, NodeFactory::Node::NodeState rawExpressionBracketEvalType);
	
	bool isOperator(const Lexeme& lexeme) const;
	OperatorEvalType getOperatorType(const Lexeme& oprLexeme) const;
	std::string printOpertatorTree(NodeFactory::NodePos tree, size_t _level = 0) const;

	std::optional<ParserNotReadyError> parserReady();
	void _ignore_parserReady();
private:
	bool checkIfValidParameterName(const std::string& parameter) const;
};