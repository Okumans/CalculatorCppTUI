#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <cassert>
#include <array>
#include <sstream>
#include <algorithm>
#include <limits>

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"
#include "nodeFactory.h"
#define DEBUG
#include "debug.cpp"
#include <iomanip>
#include <thread>
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

			if (auto result = eval.evaluateExpressionTree(rootResult); !result.isError())
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

//static std::vector<size_t> split_list_equally(const std::vector<size_t>& numbers, size_t m) {
//	size_t n = numbers.size();
//	if (n == 0) {
//		return std::vector<size_t>(m, 0);
//	}
//
//	size_t target_sum = 0;
//	for (size_t num : numbers) {
//		target_sum += num;
//	}
//	target_sum /= m;
//
//	size_t remainder = 0;
//	for (size_t num : numbers) {
//		remainder += (num % m);
//	}
//
//	std::vector<size_t> sublists_sum(m, 0);
//	size_t idx = 0;
//
//	for (size_t i = 0; i < m; i++) {
//		size_t curr_sum = 0;
//		size_t extra_element = (i < remainder);
//		size_t start_idx = idx;
//
//		while (curr_sum < target_sum + extra_element && idx < n) {
//			curr_sum += numbers[idx];
//			idx++;
//		}
//
//		sublists_sum[i] = curr_sum;
//	}
//
//	return sublists_sum;
//}
//
//void loadspliter(const Parser& pas, const std::vector<Parser::Lexeme>& load) {
//	const unsigned int coresAmount = 4;// std::thread::hardware_concurrency();
//
//	std::vector<size_t> delimeters = { 0 };
//	for (size_t i = 0; i < load.size(); i++) {
//		if (i == load.size()-1 || pas.isOperator(load[i]) && pas.getOperatorLevel(load[i]) == 0) {
//			size_t temp = delimeters.back(); delimeters.pop_back();
//			delimeters.emplace_back(i - temp);
//			delimeters.emplace_back(i);
//		}
//	}
//	delimeters.pop_back();
//
//	std::vector<size_t> splitedListSum{ split_list_equally(delimeters, coresAmount) };
//	std::vector<size_t> positions;
//	positions.reserve(splitedListSum.size());
//	positions.emplace_back(0);
//	for (auto element : splitedListSum) {
//		positions.emplace_back(positions.back() + element);
//	}
//	
//	std::cout << delimeters << "\n";
//	std::cout << positions << "\n";
//}

int main(int argc, char* argv[])
{
	NodeFactory::reserve(500);

	 //test(1'000'000); //  3.750989 second (best)
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

		// loadspliter(pas, parsedResult);

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