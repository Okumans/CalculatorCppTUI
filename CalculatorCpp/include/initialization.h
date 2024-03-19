#pragma once

#include "lexer.h"
#include "parser.h"
#include "evaluation.h"

void initializeLexer(Lexer& lexer);
void initializeParser(Parser& parser);
template<typename Floating>
void initializeEvaluator(Evaluate<Floating>& evaluator);

#include "initialization_impl.h"