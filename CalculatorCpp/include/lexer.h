#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_set>
#include "trieTree.h"
#include "result.h"

struct Brackets {
	std::unordered_map<std::string, std::string> openBracketsOperators;
	std::unordered_map<std::string, std::string> closeBracketsOperators;

	explicit Brackets(const std::vector<std::pair<std::string, std::string>>& pairs) {
		for (const auto& [open, close] : pairs) {
			openBracketsOperators[open] = close;
			closeBracketsOperators[close] = open;
		}
	}

	Brackets() = default;
};

struct LexingError {
	static const std::string prefix;
};
inline const std::string LexingError::prefix = "LexingError";

class Lexer
{
private:
	std::vector<std::string> mKeywords;
	std::unordered_set<char> mSeparatorKeys{ ' ', '\t', '\n' };
	Brackets mRawStringBracket;

	TrieTree mKeywordTree;
	TrieTree mRawStringBracketTree;

public:
	void setKeywords(const std::vector<std::string>& keywords);
	void addKeyword(const std::string& keyword);
	void setSeperatorKeys(const std::unordered_set<char>& keys);
	void addRawStringBracket(const std::string& openRawStringBracket, const std::string& closeRawStringBracket);
	void setRawStringBrackets(const std::vector <std::pair<std::string, std::string>>& RawStringBracketPairs);
	const TrieTree& getKeywordTree() const;
	static std::vector<std::string> lexing(
		const TrieTree& keywordTree,
		const TrieTree& rawStringBracketTree,
		const std::unordered_set<char>& separatorKeys,
		const Brackets& rawStringBracket,
		const std::string& currContent);
	Result<std::vector<std::string>, std::runtime_error> lexing(const std::string& currContent, bool throwError = false) const;

	void _addKeyword_not_reinitializeKeyWordTree(const std::string& keyword);
	void _reinitializeKeyWordTree();
};