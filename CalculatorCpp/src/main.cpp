#include <iostream>
#include <string>
#include <unordered_set>
#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
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

	Evaluate<long double> eval(pas);
	eval.addOperatorFunction("+", [](double a, double b) {return a + b; });
	eval.addOperatorFunction("-", [](double a, double b) {return a - b; });
	eval.addOperatorFunction("*", [](double a, double b) {return a * b; });
	eval.addOperatorFunction("/", [](double a, double b) {return a / b; });
	eval.addOperatorFunction("^", [](double a, double b) {return std::pow(a, b); });
	eval.addOperatorFunction("sqrt", [](double a) {return std::sqrt(a); });
	eval.addOperatorFunction("e", [](double a, double b) {return a * std::pow(10, b); });
	eval.addOperatorFunction("k", [](double a) {return a * 1000; });

	lex.addContent("(((4 * 2) + (6 / 3)) ^ 2 * ((9 - 2) / sqrt4) - ((7 * 3k) + (5 - 2)) + (8 / (2 ^ 2)) + ((6 + 4) * (3 - 1)) / (5 ^ 2))");
	std::cout << lex.getContent() << "\n";
	std::cout << pas.parseNumbers(lex.getContent()) << "\n";

	try {
		pas.parserReady();
		Parser::Node* root = pas.createOperatorTree(pas.parseNumbers(lex.getContent()));
		std::cout << pas.printOpertatorTree(root) << "\n";
		std::cout << "result: " << eval.evaluateExpressionTree(root) << "\n";
		
	}
	catch (const ParserSyntaxError& e) {
		std::cerr << e.what();
	}
	catch (const ParserNotReadyError& e) {
		std::cerr << e.what();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what();
	}

	return 0;
}