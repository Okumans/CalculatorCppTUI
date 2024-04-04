#include <iostream>
#include <string>
#include <unordered_set>
#include <numbers>
#include <cassert>
#include <array>
#include "lexer.h"
#include "parser.h"
#include "evaluation.h"
#include "initialization.h"
#define DEBUG
#include "debug.cpp"

const std::array<std::string, 20> one = { "", "one-", "two-", "three-", "four-",
				 "five-", "six-", "seven-", "eight-",
				 "nine-", "ten-", "eleven-", "twelve-",
				 "thirteen-", "fourteen-", "fifteen-",
				 "sixteen-", "seventeen-", "eighteen-",
				 "nineteen-" };

const std::array<std::string, 10> ten = { "", "", "twenty-", "thirty-", "forty-",
				 "fifty-", "sixty-", "seventy-", "eighty-",
				 "ninety-" };

std::string numToWords(int n, const std::string& s)
{
	std::string str = "";
	str += (n > 19) ? ten[n / 10] + one[n % 10] : one[n];
	return n ? str+s : str;
}

std::string convertToWords(long n)
{
	std::string out;
	out += numToWords((n / 10000000), "crore-");
	out += numToWords(((n / 100000) % 100), "lakh-");
	out += numToWords(((n / 1000) % 100), "thousand-");
	out += numToWords(((n / 100) % 10), "hundred-");
	if (n > 100 && n % 100)
		out += "and-";
	out += numToWords((n % 100), "");
	if (out == "")
		out = "zero";

	if (out[out.length() - 1] == '-')
		out.pop_back();

	return out;
}

int main()
{
	Lexer lex;
	initializeLexer(lex);

	Parser pas;
	initializeParser(pas);

	Evaluate<long double> eval(pas);
	initializeEvaluator(eval);

	//lex.addKeyword("<[");
	//lex.addKeyword("]>");
	//pas.addBracketOperator("<[", "]>");

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

				if (auto result = eval.evaluateExpressionTree(root.getValue()); !result.isError())
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