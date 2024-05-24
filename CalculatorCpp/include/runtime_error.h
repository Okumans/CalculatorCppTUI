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