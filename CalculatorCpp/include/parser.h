#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include "result.h"

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
		std::string value;
		Node* left{ nullptr };
		Node* right{ nullptr };

		explicit Node(const Parser::GeneralLexeme& value);
	};

	enum class OperatorEvalType {
		Prefix,
		Infix,
		Postfix,
		Constant
	};

private:
	struct {
		std::unordered_map<Parser::BracketLexeme, Parser::BracketLexeme> openBracketsOperators;
		std::unordered_map<Parser::BracketLexeme, Parser::BracketLexeme> closeBracketsOperators;
	} mBracketsOperators;
	std::unordered_map<OperatorLexeme, OperatorLevel> mOperatorLevels;
	std::unordered_map<OperatorLexeme, OperatorEvalType> mOperatorEvalTypes;
	bool mIsParserReady{ false };

public:
	static bool strictedIsNumber(const std::string& lexeme);
	Result<Node*> createOperatorTree(const std::vector<GeneralLexeme>& parsedLexemes) const;
	void setBracketOperators(const std::vector<std::pair<BracketLexeme, BracketLexeme>>& bracketPairs);
	void setOperatorLevels(const std::vector<std::pair<OperatorLexeme, OperatorLevel>>& operatorPairs);
	void setOperatorEvalType(const std::vector<std::pair<OperatorLexeme, OperatorEvalType>>& operatorEvalTypePairs);
	bool isOperator(const GeneralLexeme& lexeme) const;
	OperatorEvalType getOperatorType(const OperatorLexeme& oprLexeme) const;
	std::vector<GeneralLexeme> parseNumbers(const std::vector<GeneralLexeme>& lexemes) const;
	std::string printOpertatorTree(Parser::Node* tree);
	std::optional<ParserNotReadyError> parserReady();
	static void freeOperatorTree(Parser::Node* tree);

};