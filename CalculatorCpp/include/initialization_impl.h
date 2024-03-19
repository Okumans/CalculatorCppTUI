#ifndef INITIALIZATION_IMPL_H
#define INITIALIZATION_IMPL_H

#include "initialization.h" // Include the header file to ensure correct template instantiation

template<typename Floating>
void initializeEvaluator(Evaluate<Floating>& evaluator) {
	evaluator.addOperatorFunction("+", [](Floating a, Floating b) {return a + b; });
	evaluator.addOperatorFunction("-", [](Floating a, Floating b) {return a - b; });
	evaluator.addOperatorFunction("*", [](Floating a, Floating b) {return a * b; });
	evaluator.addOperatorFunction("/", [](Floating a, Floating b) {return a / b; });
	evaluator.addOperatorFunction("^", [](Floating a, Floating b) {return std::pow(a, b); });
	evaluator.addOperatorFunction("sqrt", [](Floating a) {return std::sqrt(a); });
	evaluator.addOperatorFunction("e+", [](Floating a, Floating b) {return a * std::pow(10, b); });
	evaluator.addOperatorFunction("k", [](Floating a) {return a * 1000; });
	evaluator.addOperatorFunction("ln", [](Floating a) {return std::log(a); });
	evaluator.addOperatorFunction("log2", [](Floating a) {return std::log2(a); });
	evaluator.addOperatorFunction("e", []() {return std::numbers::e; });
	evaluator.addOperatorFunction("pi", []() {return std::numbers::pi; });
	evaluator.addOperatorFunction("abs", [](Floating a) {return std::abs(a); });
}

#endif // INITIALIZATION_IMPL_H
