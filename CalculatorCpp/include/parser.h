#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

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
		Postfix
	};

private:
	struct {
		std::unordered_map<Parser::BracketLexeme, Parser::BracketLexeme> openBracketsOperators;
		std::unordered_map<Parser::BracketLexeme, Parser::BracketLexeme> closeBracketsOperators;
	} mBracketsOperators;
	std::unordered_map<OperatorLexeme, OperatorLevel> mOperatorLevels;
	std::unordered_map<OperatorLexeme, OperatorEvalType> mOperatorEvalTypes;

public:
	Node* createOperatorTree(const std::vector<GeneralLexeme>& parsedLexemes) const;
	void setBracketOperators(const std::vector<std::pair<BracketLexeme, BracketLexeme>>& bracketPairs);
	void setOperatorLevels(const std::vector<std::pair<OperatorLexeme, OperatorLevel>>& operatorPairs);
	void setOperatorEvalType(const std::vector<std::pair<OperatorLexeme, OperatorEvalType>>& operatorEvalTypePairs);
	std::vector<GeneralLexeme> parseNumbers(const std::vector<GeneralLexeme>& lexemes) const;
	std::string printOpertatorTree(Parser::Node* tree);
};
