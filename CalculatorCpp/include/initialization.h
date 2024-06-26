#pragma once

#include <functional>

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"


#ifndef N_LEXER
void initializeLexer(Lexer& lexer);
std::function<std::vector<std::string>(const std::string&)> initializeStaticLexer(const std::vector<std::string>& extendsKeywords);
std::function<std::vector<std::string>(const std::string&)> initializeStaticLexer(const std::unordered_set<std::string>& extendsKeywords);
#endif // N_LEXER

#ifndef N_PARSER
void initializeParser(Parser& parser);
#endif // N_PARSER

#ifndef N_EVALUATE
template<typename Floating>
void initializeEvaluator(Evaluate& evaluator);
#endif // N_EVALUATE



#if !defined(N_LEXER) && !defined(N_PARSER) && !defined(N_EVALUATE)
//template<typename Floating = long double>
//class OperatorDefiner
//{
//private:
//	Lexer& mLexer;
//	Parser& mParser;
//	Evaluate<Floating>& mEvaluate;
//
//public:
//	using FloatingType = Floating;
//	OperatorDefiner(Lexer& lexer, Parser& parser, Evaluate<Floating>& evaluate) : mLexer{ lexer }, mParser{ parser }, mEvaluate{ evaluate } {}
//	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating()>& operatorDefinition);
//	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating)>& operatorDefinition);
//	void defineOperator(std::string operatorName, Parser::OperatorLevel operatorLevel, Parser::OperatorEvalType operatorEvalType, const std::function<Floating(Floating, Floating)>& operatorDefinition);
//};
#endif

#include "initialization_impl.h"