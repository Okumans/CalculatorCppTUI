#include <numbers>
#include <vector>
#include <unordered_set>
#include <string>
#include <tuple>
#include <queue>
#include "initialization.h"
#include "nodeFactory.h"

const std::unordered_set<char> mainSeparatorKeys{ ' ', '\n', '\t' };
const std::vector<std::string> mainRawStringBracket{ "{", "}", "[", "]" };
const std::vector<std::string> mainKeywords{
	"+",
	"-",
	"*",
	"/",
	//"//",
	//"!",
	"^",
	"sqrt",
	//"abs",
	"[",
	"]",
	"(",
	")",
	"{",
	"}",
	".",
	//"k",
	//"ln",
	//"log2",
	//"~",
	"e",
	//"pi",
	//"setmem",
	//"readmem"
};

static std::vector<std::pair<std::string, std::string>> splitIntoPairs(const std::vector<std::string>& vec) {
	std::vector<std::pair<std::string, std::string>> result;
	result.reserve(vec.size() / 2 + vec.size() % 2); 
	for (size_t i = 0; i < vec.size(); i += 2) {
		if (i + 1 < vec.size()) 
			result.emplace_back(vec[i], vec[i + 1]);
		else 
			result.emplace_back(vec[i], "");
	}
	return result;
}

void initializeLexer(Lexer& lexer) {
	lexer.addRawStringBracket("{", "}");
	lexer.addRawStringBracket("[", "]");
	lexer.setKeywords(mainKeywords);
	lexer.setSeperatorKeys(mainSeparatorKeys);
}

std::function<std::vector<std::string>(const std::string&)> initializeStaticLexer(const std::vector<std::string> &extendsKeywords) {
	static TrieTree keywordTree(mainKeywords);
	static TrieTree rawStringBracketKeywordTree(mainRawStringBracket);
	static Brackets rawStringBracket{splitIntoPairs(mainRawStringBracket)};
	static std::queue<std::string> skeduleRemove;

	while (!skeduleRemove.empty()) {
		keywordTree.remove(skeduleRemove.front());
		skeduleRemove.pop();
	}

	for (const std::string& extendsKeyword : extendsKeywords) {
		keywordTree.insert(extendsKeyword);
		skeduleRemove.emplace(extendsKeyword);
	}

	return [](const std::string& content) {return Lexer::lexing(keywordTree, rawStringBracketKeywordTree, mainSeparatorKeys, rawStringBracket, content); };
}

std::function<std::vector<std::string>(const std::string&)> initializeStaticLexer(const std::unordered_set<std::string>& extendsKeywords) {
	static TrieTree keywordTree(mainKeywords);
	static TrieTree rawStringBracketKeywordTree(mainRawStringBracket);
	static Brackets rawStringBracket{ splitIntoPairs(mainRawStringBracket) };
	static std::queue<std::string> skeduleRemove;

	while (!skeduleRemove.empty()) {
		keywordTree.remove(skeduleRemove.front());
		skeduleRemove.pop();
	}

	for (const std::string& extendsKeyword : extendsKeywords) {
		keywordTree.insert(extendsKeyword);
		skeduleRemove.emplace(extendsKeyword);
	}

	return [](const std::string& content) {return Lexer::lexing(keywordTree, rawStringBracketKeywordTree, mainSeparatorKeys, rawStringBracket, content); };
}


void initializeParser(Parser& parser) {
	const std::vector<std::pair<Parser::Lexeme, Parser::Lexeme>> mainBracketPairs{
		{"[", "]"},
		{"(", ")"},
		{"{", "}"},
	};

	const std::vector<std::pair<Parser::Lexeme, Parser::OperatorLevel>> mainOperatorLevels{
		{"+", 0},
		{"-", 0},
		{"*", 1},
		{"/", 1},
		//{"//", 1},
		//{"!", 9},
		//{"~", 9},
		{"^", 2},
		{"sqrt", 9},
		//{"abs", 9},
		//{"k", 9},
		//{"ln", 9},
		//{"log2", 9},
		{"e", 9},
		//{"pi", 9},
		//{"readmem", 9},
		//{"setmem", 9},
	};

	using EvalType = Parser::OperatorEvalType;
	const std::vector<std::pair<Parser::Lexeme, Parser::OperatorEvalType>> mainOperatorEvalType{
		{"+", EvalType::Infix},
		{"-", EvalType::Infix},
		{"*", EvalType::Infix},
		{"/", EvalType::Infix},
		//{"//", EvalType::Infix},
		//{"!", EvalType::Prefix},
		//{"~", EvalType::Postfix},
		{"^", EvalType::Infix},
		{"sqrt", EvalType::Postfix},
		//{"abs", EvalType::Postfix},
		//{"k", EvalType::Prefix},
		//{"ln", EvalType::Postfix},
		//{"log2", EvalType::Postfix},
		{"e", EvalType::Constant},
		//{"pi", EvalType::Constant},
		//{"readmem", EvalType::Postfix},
		//{"setmem", EvalType::Infix},
	};

	parser.setBracketOperators(mainBracketPairs);
	parser.setOperatorLevels(mainOperatorLevels);
	parser.setOperatorEvalType(mainOperatorEvalType);

	parser.setRawExpressionBracketEvalType({ {"{", NodeFactory::Node::NodeState::LambdaFuntion} });
	parser.setRawExpressionBracketEvalType({ {"[", NodeFactory::Node::NodeState::Storage} });
}