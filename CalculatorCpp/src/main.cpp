#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <cassert>
#include <array>
#include <sstream>
#include <limits>

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"
#include "nodeFactory.h"
#define DEBUG
#include "debug.cpp"

#include <iomanip>

int main(int argc, char* argv[])
{
	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate<double> eval(pas);
	initializeEvaluator(eval);

	std::string input{};
	if (argc >= 2) {
		input = argv[1];
	}

	size_t count{ 0 };
	while (++count && (argc < 2 || count == 1))
	{
		std::cout << "\n## EXPRESSION: " << count << "##\n";
		std::cout << "Expression = ";

		!(argc >= 2) && std::getline(std::cin, input);

		if (input == "quit")
			return 0;

		auto lexResult = lex.lexing(input);

		std::stringstream ss;
		ss << lexResult;

		std::cout << "Lexing: " << ss.str().substr(0, 1000) << "\n";

		auto parsedResult = pas.parseNumbers(lexResult);

		ss.str("");
		ss.clear();

		ss << parsedResult;

		std::cout << "Parsing Number: " << ss.str().substr(0, 1000) << "\n";

		if (!pas.parserReady().has_value()) {
			auto root = pas.createOperatorTree(parsedResult);

			if (!root.isError()) {
				auto rootResult = root.getValue();
				auto rootVal = std::get<NodeFactory::NodePos>(rootResult);
				std::cout << "Operation Tree: " << pas.printOpertatorTree(rootVal) << "\n";

				if (auto result = eval.evaluateExpressionTree(rootVal); !result.isError())
					std::cout << "Result: " << std::fixed << result.getValue() << "\n";
				else
					std::cout << "ERROR: " << result.getException().what() << "\n";

				NodeFactory::freeAll();
			}
			else {
				std::cout << "ERROR: " << root.getException().what() << "\n";
			}
		}
	}

	return 0;
}