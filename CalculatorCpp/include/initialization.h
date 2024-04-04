#pragma once

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"

void initializeLexer(Lexer& lexer);
void initializeParser(Parser& parser);
template<typename Floating>
void initializeEvaluator(Evaluate<Floating>& evaluator);

template<typename Floating = long double>
class OperatorDefiner
{
private:
	Lexer& mLexer;
	Parser& mParser;
	Evaluate<Floating>& mEvaluate;

public:
	using FloatingType = Floating;
	OperatorDefiner(Lexer& lexer, Parser& parser, Evaluate<Floating>& evaluate) : mLexer{ lexer }, mParser{ parser }, mEvaluate{ evaluate } {}
	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating()>& operatorDefinition);
	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating)>& operatorDefinition);
	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating, Floating)>& operatorDefinition);
};

#include "initialization_impl.h"