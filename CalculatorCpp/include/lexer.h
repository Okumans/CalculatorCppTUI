#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_set>
#include "trieTree.h"

class Lexer
{
private:
	std::string mRawContent;
	std::vector<std::string> mContent;
	std::vector<std::string> mKeywords;
	std::unordered_set<char> mSeparatorKeys{ ' ', '\t', '\n' };

	struct {
		std::unordered_map<std::string, std::string> openBracketsOperators;
		std::unordered_map<std::string, std::string> closeBracketsOperators;
	} mRawStringBracket;

	TrieTree mKeywordTree;
	TrieTree mRawStringBracketTree;

public:
	Lexer();
	explicit Lexer(const std::string& raw_content);
	void addContent(const std::string& content);
	void setKeywords(const std::vector<std::string>& keywords);
	void addKeyword(const std::string& keyword);
	void setSeperatorKeys(const std::unordered_set<char>& keys);
	void addRawStringBracket(const std::string& openRawStringBracket, const std::string& closeRawStringBracket);
	void setRawStringBrackets(const std::vector <std::pair<std::string,std::string>>& RawStringBracketPairs);
	void reLexing();
	const std::vector<std::string>& getContent() const;
	const std::string& getRawContent() const;
	const TrieTree& getKeywordTree() const;
	static std::vector<std::string> lexing(const TrieTree& keywordTree, const std::unordered_set<char>& separatorKeys, const std::string& currContent);
	std::vector<std::string> lexing(const std::string& currContent) const;
	
	void _addKeyword_not_reinitializeKeyWordTree(const std::string& keyword);
	void _reinitializeKeyWordTree();
};