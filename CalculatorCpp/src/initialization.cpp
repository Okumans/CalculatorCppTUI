#include <numbers>
#include <vector>
#include <unordered_set>
#include <string>
#include <tuple>
#include "initialization.h"

void initializeLexer(Lexer& lexer) {
	const std::unordered_set<char> mainSeparatorKeys{ ' ', '\n', '\t' };
	const std::vector<std::string> mainKeywords{
		"+",
		"-",
		"*",
		"/",
		"//",
		"^",
		"e+",
		"sqrt",
		"abs",
		"[",
		"]",
		"(",
		")",
		"{",
		"}",
		".",
		"k",
		"ln",
		"log2",
		"e",
		"pi",
	};

	lexer.setKeywords(mainKeywords);
	lexer.setSeperatorKeys(mainSeparatorKeys);
}

void initializeParser(Parser& parser) {
	const std::vector<std::pair<Parser::BracketLexeme, Parser::BracketLexeme>> mainBracketPairs{
		{"[", "]"},
		{"(", ")"},
		{"{", "}"}
	};

	const std::vector<std::pair<Parser::OperatorLexeme, Parser::OperatorLevel>> mainOperatorLevels{
		{"+", 0},
		{"-", 0},
		{"*", 1},
		{"/", 1},
		{"//", 1},
		{"e+", 2},
		{"^", 2},
		{"sqrt", 9},
		{"abs", 9},
		{"k", 9},
		{"ln", 9},
		{"log2", 9},
		{"e", 9},
		{"pi", 9},
	};

	using EvalType = Parser::OperatorEvalType;
	const std::vector<std::pair<Parser::OperatorLexeme, Parser::OperatorEvalType>> mainOperatorEvalType{
		{"+", EvalType::Infix},
		{"-", EvalType::Infix},
		{"*", EvalType::Infix},
		{"/", EvalType::Infix},
		{"//", EvalType::Infix},
		{"e+", EvalType::Infix},
		{"^", EvalType::Infix},
		{"sqrt", EvalType::Postfix},
		{"abs", EvalType::Postfix},
		{"k", EvalType::Prefix},
		{"ln", EvalType::Postfix},
		{"log2", EvalType::Postfix},
		{"e", EvalType::Constant},
		{"pi", EvalType::Constant},
	};

	parser.setBracketOperators(mainBracketPairs);
	parser.setOperatorLevels(mainOperatorLevels);
	parser.setOperatorEvalType(mainOperatorEvalType);
}