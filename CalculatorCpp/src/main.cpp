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
#include <chrono>

#define BENCHMARK_START auto start = std::chrono::steady_clock::now()
#define BENCHMARK_END auto end = std::chrono::steady_clock::now(); \
                      std::chrono::duration<double> duration = end - start; \
                      std::cout << "Time taken: " << duration.count() << " seconds\n"


void test(size_t basicOperationAmount) {
	std::string buffer;
	for (size_t i{ 0 }; i < basicOperationAmount; i++)
		buffer += "1+";
	buffer += "1";

	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate eval(pas);
	initializeEvaluator(eval);


	std::cout << "LEXER benckmark (" << basicOperationAmount << " operations) -> ";
	std::vector<Parser::Lexeme> lexResult;
	{
		BENCHMARK_START;
		lexResult = lex.lexing(buffer);
		BENCHMARK_END;
	}


	std::cout << "PARSER benckmark (" << basicOperationAmount << " operations) -> ";
	std::vector<Parser::Lexeme> parsedResult;
	{
		BENCHMARK_START;
		parsedResult = pas.parseNumbers(lexResult);
		BENCHMARK_END;
	}


	std::cout << "EVALUATOR benckmark (" << basicOperationAmount << " operations) -> ";
	BENCHMARK_START;
	if (!pas.parserReady().has_value()) {
		auto root = pas.createOperatorTree(parsedResult);

		if (!root.isError()) {
			auto rootResult = root.moveValue();
			auto rootVal = rootResult;

			if (auto result = eval.evaluateExpressionTree(rootVal); !result.isError())
				std::cout << "Result: " << std::fixed << result.getValue() << ", ";
			else
				std::cout << "ERROR: " << result.getException().what() << ", ";

			NodeFactory::freeAll();
		}
		else {
			std::cout << root.getException().what() << ", ";
		}
	}
	BENCHMARK_END;
}


int main(int argc, char* argv[])
{
	 //test(1'000'000); // 4.87 second
	 //return 0;

	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate eval(pas);
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

		getReturnType(NodeFactory::NodePosNull, {}, false); // reset cache

		if (!pas.parserReady().has_value()) {
			auto root = pas.createOperatorTree(parsedResult);

			if (!root.isError()) {
				auto rootResult = root.getValue();
				const auto &rootVal = rootResult;
				// std::cout << "Operation Tree: " << pas.printOpertatorTree(rootVal) << "\n";

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