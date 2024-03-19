#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"

#define DEBUG
#include "debug.cpp"


int main()
{
	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate<long double> eval(pas);
	initializeEvaluator(eval);

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

				if (auto result{ eval.evaluateExpressionTree(root.getValue()) }; !result.isError())
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
