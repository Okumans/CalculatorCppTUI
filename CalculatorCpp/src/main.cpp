#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <cassert>
#include <array>

#include <limits>

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

	OperatorDefiner oprDef(lex, pas, eval);

	using FloatingType = decltype(oprDef)::FloatingType;
	std::unique_ptr<FloatingType[]> memory{ new FloatingType[1000] };

	oprDef.defineOperator("writemem", 9, Parser::OperatorEvalType::Infix, [&memory](FloatingType memValue, FloatingType memAddress) {
		if (0 <= memAddress && memAddress < 10000) {
			memory[(int)memAddress] = memValue;
			return 1;
		}
		return 0;
		});

	oprDef.defineOperator("readmem", 9, Parser::OperatorEvalType::Postfix, [&memory](FloatingType memAddress) {
		if (0 <= memAddress && memAddress < 10000) {
			return memory[(int)memAddress];
		};
		return std::numeric_limits<FloatingType>::max();
		});

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
				auto rootResult = root.getValue();
				auto rootVal = std::get<Parser::Node*>(rootResult);
				std::cout << "Operation Tree: " << pas.printOpertatorTree(rootVal) << "\n";

				if (auto result = eval.evaluateExpressionTree(rootVal); !result.isError())
					std::cout << "Result: " << result.getValue() << "\n";
				else
					std::cout << "ERROR: " << result.getException().what() << "\n";

				Parser::freeOperatorTree(rootVal);
			}
			else {
				std::cout << "ERROR: " << root.getException().what() << "\n";
			}
		}
	}

	return 0;
}