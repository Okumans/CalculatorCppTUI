#pragma once
#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

#include "result.h"
#include "lexer.h"
#include "runtimeType.h"
#include "runtimeTypedExprComponent.h"
#include "nodeFactory.h"

struct ParserNotReadyError {
	static const std::string prefix;
};
inline const std::string ParserNotReadyError::prefix = "ParserNotReadyError";

struct ParserSyntaxError {
	static const std::string prefix;
};
inline const std::string ParserSyntaxError::prefix = "ParserSyntaxError";


class Parser {
public:
	using OperatorLevel = size_t;
	using Lexeme = std::string;

	enum class OperatorEvalType : int8_t {
		Infix,
		Postfix,
		Prefix,
		Constant,
	};

private:
	Brackets mBracketsOperators;
	std::unordered_map<Lexeme, OperatorLevel> mOperatorLevels;
	std::unordered_map<Lexeme, Lambda>& mEvaluatorLambdaFunction;
	std::unordered_map<Lexeme, NodeFactory::Node::NodeState> mRawExpressionBracketEvalTypes;
	bool mIsParserReady{ false };

public:
	Parser(std::unordered_map<Parser::Lexeme, Lambda>& EvaluatorLambdaFunctions);
	static bool strictedIsNumber(const std::string& lexeme, bool veryStrict = false);

	// main functions
	std::vector<Lexeme> parseNumbers(const std::vector<Lexeme>& lexemes) const;
	Result<std::vector<NodeFactory::NodePos>> createOperatorTree(const std::vector<Lexeme>& parsedLexemes, std::unordered_map<Lexeme, Lambda>& EvaluatorLambdaFunction) const;
	Result<NodeFactory::NodePos> createRawExpressionOperatorTree(const std::string& RawExpression, NodeFactory::Node::NodeState RawExpressionType, std::unordered_map<Lexeme, Lambda>& EvaluatorLambdaFunction) const;
	NodeFactory::NodePos createRawExpressionStorage(const std::vector<NodeFactory::NodePos>& parsedExpressions) const;
	
	// setters
	void setBracketOperators(const std::vector<std::pair<Lexeme, Lexeme>>& bracketPairs);
	void setOperatorLevels(const std::vector<std::pair<Lexeme, OperatorLevel>>& operatorPairs);
	void addOperatorLevel(const Lexeme& operatorLexeme, OperatorLevel operatorLevel);
	void addBracketOperator(const Lexeme& openBracket, const Lexeme& closeBracket);
	void setRawExpressionBracketEvalType(const std::vector<std::pair<Lexeme, NodeFactory::Node::NodeState>>& rawExpressionBracketEvalTypePairs);
	void addRawExpressionBracketEvalType(const Lexeme& openBracketLexeme, NodeFactory::Node::NodeState rawExpressionBracketEvalType);
	
	bool isOperator(const Lexeme& lexeme) const;
	Lambda::LambdaNotation getNotation(const Lexeme& oprLexeme) const;
	OperatorLevel getOperatorLevel(const Lexeme& oprLexeme) const;
	std::string printOpertatorTree(std::vector<NodeFactory::NodePos> trees, const std::unordered_map<Parser::Lexeme, Lambda>& EvaluatorLambdaFunctions) const;

	std::optional<RuntimeError<ParserNotReadyError>> parserReady();
	void _ignore_parserReady();
private:
	std::unordered_set<Lexeme> mTempConstant;
	std::optional<std::runtime_error> getLambdaType(std::vector<std::pair<std::string, RuntimeType>>& parametersWithTypes, std::string parameterExpression) const;
	bool checkIfValidParameterName(const std::string& parameter) const;
};