#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "trieTree.h"

class Lexer
{
private:
	std::string mRawContent;
	std::vector<std::string> mContent;
	std::vector<std::string> mKeywords;
	std::unordered_set<char> mSeparatorKeys{ ' ', '\t', '\n' };
	TrieTree mKeywordTree;

public:
	Lexer();
	explicit Lexer(const std::string& raw_content);
	void addContent(const std::string& content);
	void setKeywords(const std::vector<std::string>& keywords);
	void setSeperatorKeys(const std::unordered_set<char>& keys);
	void reLexing();
	const std::vector<std::string>& getContent() const;
	const std::string& getRawContent() const;
	const TrieTree& getKeywordTree() const;
	static std::vector<std::string> lexing(const TrieTree& keywordTree, const std::unordered_set<char>& separatorKeys, const std::string& currContent);
	std::vector<std::string> lexing(const std::string& currContent) const;

private:
	void reinitializeKeyWordTree();
};