#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
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
	eval.addOperatorFunction("e+", [](double a, double b) {return a * std::pow(10, b); });
	eval.addOperatorFunction("k", [](double a) {return a * 1000; });
	eval.addOperatorFunction("ln", [](double a) {return std::log(a); });
	eval.addOperatorFunction("log2", [](double a) {return std::log2(a); });
	eval.addOperatorFunction("e", []() {return std::numbers::e; });
	eval.addOperatorFunction("pi", []() {return std::numbers::pi; });
	eval.addOperatorFunction("abs", [](double a) {return std::abs(a); });



	size_t count{ 0 };
	while (++count)
	{
		std::cout << "\n## EXPRESSION: " << count << "##\n";
		std::cout << "Expression = ";
		std::string input{};
		
		std::getline(std::cin, input);

		if (input == "quit")
			return 0;

		auto lexResult = lex.lexing(input);
		std::cout << "Lexing: " << lexResult << "\n";

		auto parsedResult = pas.parseNumbers(lexResult);
		std::cout << "Parsing Number: " << parsedResult << "\n";


		if (!pas.parserReady().has_value()) {
			auto root = pas.createOperatorTree(parsedResult);

			if (!root.isError()) {
				std::cout << "Operation Tree: " << pas.printOpertatorTree(root.getValue()) << "\n";

				auto result = eval.evaluateExpressionTree(root.getValue());
				if (!result.isError())
					std::cout << "Result: " << result.getValue() << "\n";
				else
					std::cout << "ERROR: " << result.getException().what() << "\n";
				
				Parser::freeOperatorTree(root.getValue());
			}
			else {
				std::cout << "ERROR: " << root.getException().what() << "\n";
			}

		}
	}


	return 0;
}