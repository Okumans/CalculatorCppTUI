#pragma once

#include <format>
#include "colorText.h"

template<typename ErrorType>
class RuntimeError : public std::runtime_error {
public:
	explicit RuntimeError(const std::string& message)
		: std::runtime_error(HighlightSyntax(std::format("{}: {}", ColorText<Color::Red>(ErrorType::prefix), message))) {}

	explicit RuntimeError(const std::string& message, const std::string& from)
		: std::runtime_error(HighlightSyntax(std::format("{}: {} [{}]", ColorText<Color::Red>(ErrorType::prefix), message, ColorText<Color::Magenta>(from)))) {}

	explicit RuntimeError(const std::runtime_error& baseError, const std::string& message, const std::string& from)
		: std::runtime_error(HighlightSyntax(std::format("{}: {} [{}]\n {} {}", ColorText<Color::Red>(ErrorType::prefix), message, ColorText<Color::Magenta>(from), ColorText<Color::Yellow>("|--"), baseError.what()))) {}
};

struct RuntimeTypeError {
	static const std::string prefix;
};
inline const std::string RuntimeTypeError::prefix = "RuntimeTypeError";

struct LambdaConstructionError {
	static const std::string prefix;
};
inline const std::string LambdaConstructionError::prefix = "LambdaConstructionError";

struct LambdaEvaluationError {
	static const std::string prefix;
};
inline const std::string LambdaEvaluationError::prefix = "LambdaEvaluationError";

struct StorageEvaluationError {
	static const std::string prefix;
};
inline const std::string StorageEvaluationError::prefix = "StorageEvaluationError";
