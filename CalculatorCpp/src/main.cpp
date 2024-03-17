#include <iostream>
#include <string>
#include <unordered_set>
#include "lexer.h"
#include "parser.h"
#define DEBUG
#include "debug.cpp"

static std::string_view trimLeadingZeros(const std::string& str) {
	size_t start = str.find_first_not_of('0');
	size_t sep = str.find('.');
	if (start != std::string::npos)
		return std::string_view(str.c_str() + start - (sep >= start && sep != std::string::npos ? 1 : 0), (str.length() - start));
	return std::string_view(str);
}

int main()
{
	const std::unordered_set<char> mainSeparatorKeys{ ' ', '\n', '\t' };
	const std::vector<std::string> mainKeywords{
		"+",
		"-",
		"*",
		"/",
		"//",
		"^",
		"e",
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
	};

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
		{"e", 2},
		{"^", 2},
		{"sqrt", 9},
		{"abs", 9},
		{"k", 9},
	};

	using EvalType = Parser::OperatorEvalType;
	const std::vector<std::pair<Parser::OperatorLexeme, Parser::OperatorEvalType>> mainOperatorEvalType{
		{"+", EvalType::Infix},
		{"-", EvalType::Infix},
		{"*", EvalType::Infix},
		{"/", EvalType::Infix},
		{"//", EvalType::Infix},
		{"e", EvalType::Infix},
		{"^", EvalType::Infix},
		{"sqrt", EvalType::Postfix},
		{"abs", EvalType::Postfix},
		{"k", EvalType::Prefix},
	};

	Lexer lex;
	lex.setKeywords(mainKeywords);
	lex.setSeperatorKeys(mainSeparatorKeys);

	Parser pas;
	pas.setBracketOperators(mainBracketPairs);
	pas.setOperatorLevels(mainOperatorLevels);
	pas.setOperatorEvalType(mainOperatorEvalType);

	lex.addContent("sqrt(4) + abs(-5-10) - 10 * 3k / 2^3 // 3e2");
	std::cout << lex.getContent() << "\n";
	std::cout << pas.parseNumbers(lex.getContent()) << "\n";

	Parser::Node* root = pas.createOperatorTree(pas.parseNumbers(lex.getContent()));
	std::cout << pas.printOpertatorTree(root) << "\n";
	

	return 0;
}