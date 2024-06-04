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

	Evaluate eval;
	initializeEvaluator(eval);

	Parser pas(eval.getEvaluationLambdaFunction());
	initializeParser(pas);



	std::cout << "LEXER benckmark (" << basicOperationAmount << " operations) -> ";
	std::vector<Parser::Lexeme> lexResult;
	{
		BENCHMARK_START;
		lexResult = lex.lexing(buffer).getValue();
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
		auto root = pas.createOperatorTree(parsedResult, eval.getEvaluationLambdaFunction());

		if (!root.isError()) {
			auto rootResult = root.moveValue();

			if (auto result = eval.evaluateExpressionTree(rootResult); !result.isError())
				std::cout << "Result: " << std::fixed << result.getValue() << ", ";
			else
				std::cout << result.getException().what() << ", ";

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

	 //test(1'000'000); //  2.1 second (best)
	 //return 0;

	Lexer lex;
	initializeLexer(lex);

	Evaluate eval;
	initializeEvaluator(eval);

	Parser pas(eval.getEvaluationLambdaFunction());
	initializeParser(pas);



	std::string input{};
	if (argc >= 2) {
		input = argv[1];
	}

	size_t count{ 0 };
	while (++count && (argc < 2 || count == 1))
	{
		//std::cout << "\n## EXPRESSION: " << count << "##\n";
		std::cout << ColorText<Color::Magenta>("$: ");

		!(argc >= 2) && std::getline(std::cin, input);

		if (input == "quit")
			return 0;

		// debug tool make it better later
		if (input == ":node-profile") {
			size_t nodePosition{ 0 };
			while (NodeFactory::validNode(nodePosition)) {
				const NodeFactory::Node& nNode{ NodeFactory::node(nodePosition) };
				std::string utilityStorageString;
				for (const auto& [paramName, paramType] : nNode.parametersWithType) {
					utilityStorageString += paramName + ": " + RuntimeTypeToString(paramType) + ", ";
				}
				if (nNode.parametersWithType.size()) {
					utilityStorageString.pop_back();
					utilityStorageString.pop_back();
				}
				std::cout << std::format("Node({}): \n\t value-----\t: \"{}\" \n\t nodeState-\t: {}\n\t leftPos---\t: {} \n\t rightPos--\t: {} \n\t paramsType\t: [{}]\n", nodePosition, nNode.value, (int)nNode.nodeState, nNode.leftPos, nNode.rightPos, utilityStorageString);
				nodePosition++;
			}
		}

		auto lexResult = lex.lexing(input, true);

		if (lexResult.isError()) {	
			std::cout << lexResult.getException().what() << "\n\n";
			continue;
		}

		//std::stringstream ss;
		//ss << lexResult;

		//std::cout << "Lexing: " << HighlightSyntax(ss.str().substr(0, 1000)) << "\n";

		auto parsedResult = pas.parseNumbers(lexResult.getValue());

		// loadspliter(pas, parsedResult);

		//ss.str("");
		//ss.clear();

		//ss << parsedResult;

		//std::cout << "Parsing Number: " << HighlightSyntax(ss.str().substr(0, 1000)) << "\n";

		// getReturnType(NodeFactory::NodePosNull, {}, false); // reset cache

		if (!pas.parserReady().has_value()) {
			auto root = pas.createOperatorTree(parsedResult, eval.getEvaluationLambdaFunction());

			if (!root.isError()) {
				auto rootResult{ root.moveValue() };
				std::cout << ColorText<Color::Yellow>(" ⇒ ") << pas.printOpertatorTree(rootResult, eval.getEvaluationLambdaFunction()) << "\n";

				BENCHMARK_START;
				if (auto result = eval.evaluateExpressionTree(rootResult); !result.isError())
					std::cout << ColorText<Color::Green>(" ≡ ") << std::fixed << HighlightSyntax(result.getValue().toString()) << "\n";
				else
					std::cout << ColorText<Color::Red>(" (!) ") << HighlightSyntax(result.getException().what()) << "\n";
				BENCHMARK_END;

				// NodeFactory::freeAll();
			}
			else {
				std::cout << ColorText<Color::Red>("(!) ") << HighlightSyntax(root.getException().what()) << "\n";
			}
		}

		std::cout << "Node amount: " << NodeFactory::size() << "\n";
		std::cout << std::endl;
	}

	return 0;
}