#pragma once

#include <array>
#include <string_view>
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <boost/regex.hpp>

// ANSI escape codes for colors
constexpr std::array color_codes = {
	"\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m",
	"\033[35m", "\033[36m", "\033[37m", "\033[90m", "\033[91m",
	"\033[92m", "\033[93m", "\033[94m", "\033[95m", "\033[96m",
	"\033[97m", "\033[0m"
};

// Color enumeration
enum class Color {
	Black, Red, Green, Yellow, Blue, Magenta, Cyan, White,
	Bright_Black, Bright_Red, Bright_Green, Bright_Yellow,
	Bright_Blue, Bright_Magenta, Bright_Cyan, Bright_White,
	Reset
};

// ColorText function (for string literals)
template<Color color>
constexpr std::string ColorText(const char* text) {
	std::string result = color_codes[static_cast<std::size_t>(color)];
	result += text;
	result += color_codes[static_cast<std::size_t>(Color::Reset)];
	return result;
}

// ColorText function (for std::string)
template<Color color>
constexpr std::string ColorText(const std::string& text) {
	return color_codes[static_cast<std::size_t>(color)] + text + color_codes[static_cast<std::size_t>(Color::Reset)];
}

// ColorText function (for char)
template<Color color>
constexpr std::string ColorText(const char text) {
	std::string result;
	result += color_codes[static_cast<std::size_t>(color)];
	result += text;
	result += color_codes[static_cast<std::size_t>(Color::Reset)];
	return result;
}

// Syntax highlighting function
inline std::string HighlightSyntax(const std::string& code) {
	// Define regex patterns for keywords, numbers, and strings
	const boost::regex bracket_contents_regex(R"((?<=\().*?(?=\)))");
	const boost::regex bracket_regex(R"((?<!\033)[\[\]\(\)\{\}])");
	const boost::regex type_regex(R"(\b(Number|Storage|Lambda)\b)");
	const boost::regex number_regex(R"(\b\d+(\.\d+)?\b)");
	const boost::regex operators_regex(R"([=!+\-#@$%^&*?",])");

	std::string highlighted_code = code;

	//highlighted_code = boost::regex_replace(highlighted_code, bracket_contents_regex, ColorText<Color::Bright_Yellow>("$&"));

	// Highlight numbers
	highlighted_code = boost::regex_replace(highlighted_code, number_regex, ColorText<Color::Bright_Blue>("$&"));

	// Highlight types
	highlighted_code = boost::regex_replace(highlighted_code, type_regex, ColorText<Color::Green>("$&"));

	// Highlight types
	highlighted_code = boost::regex_replace(highlighted_code, operators_regex, ColorText<Color::Cyan>("$&"));

	// Highlight brackets
	highlighted_code = boost::regex_replace(highlighted_code, bracket_regex, ColorText<Color::Yellow>("$&"));
	return highlighted_code;
}