#include "lexer.h"
#include <cstddef>
#include <vector>
#include <iterator>
#include <cctype>
#include <stack>

#define DEBUG
#include "debug.cpp"

static void nonEmptyPushback(std::vector<std::string>& vec, const std::string& str)
{
	if (!str.empty()) vec.push_back(str);
}

void Lexer::setKeywords(const std::vector<std::string>& keywords)
{
	mKeywords = keywords;
	_reinitializeKeyWordTree();
}

void Lexer::addKeyword(const std::string& keyword)
{
	mKeywords.push_back(keyword);
	_reinitializeKeyWordTree();
}

void Lexer::_addKeyword_not_reinitializeKeyWordTree(const std::string& keyword)
{
	mKeywords.push_back(keyword);
}

void Lexer::setSeperatorKeys(const std::unordered_set<char>& keys) {
	mSeparatorKeys = keys;
}

void Lexer::addRawStringBracket(const std::string& openRawStringBracket, const std::string& closeRawStringBracket)
{
	mRawStringBracket.closeBracketsOperators[closeRawStringBracket] = openRawStringBracket;
	mRawStringBracket.openBracketsOperators[openRawStringBracket] = closeRawStringBracket;

	if (!mRawStringBracketTree.search(closeRawStringBracket))
		mRawStringBracketTree.insert(closeRawStringBracket);
	if (!mRawStringBracketTree.search(openRawStringBracket))
		mRawStringBracketTree.insert(openRawStringBracket);
}

void Lexer::setRawStringBrackets(const std::vector<std::pair<std::string, std::string>>& RawStringBracketPairs)
{
	for (const auto& [openRawStringBracket, closeRawStringBracket] : RawStringBracketPairs)
	{
		mRawStringBracket.closeBracketsOperators[closeRawStringBracket] = openRawStringBracket;
		mRawStringBracket.openBracketsOperators[openRawStringBracket] = closeRawStringBracket;

		if (!mRawStringBracketTree.search(closeRawStringBracket))
			mRawStringBracketTree.insert(closeRawStringBracket);
	}
}

const TrieTree& Lexer::getKeywordTree() const {
	return mKeywordTree;
}

bool handleRawString(
	char chr,
	std::vector<std::string>& temp,
	std::string& buff,
	std::string& rawStringBracketBuff,
	TrieTree::StartsWithsInstance& startWithInst,
	TrieTree::StartsWithsInstance& startWithRawStringInst,
	std::stack<std::string>& isCurrentRawString,
	const Brackets& rawStringBracket) {
	if (!buff.empty() && isCurrentRawString.empty()) {
		isCurrentRawString.emplace(temp.back());
		startWithRawStringInst.reset();
		startWithInst.reset();

		if (startWithRawStringInst.insertChar(buff.back())) {
			rawStringBracketBuff += buff.back();
			buff.pop_back();
		}
	}

	if (startWithRawStringInst.insertChar(chr)) {
		rawStringBracketBuff += chr;
		return true;
	}

	else if (!rawStringBracketBuff.empty() &&
		rawStringBracket.openBracketsOperators.contains(temp.back()) &&
		rawStringBracket.openBracketsOperators.at(temp.back()) == rawStringBracketBuff)
	{
		isCurrentRawString.pop();
		if (isCurrentRawString.empty()) {
			nonEmptyPushback(temp, buff);
			nonEmptyPushback(temp, rawStringBracketBuff);
			buff.clear();
			rawStringBracketBuff.clear();
		}
		else {
			buff += rawStringBracketBuff;

			startWithRawStringInst.reset();
			rawStringBracketBuff.clear();

			if (startWithRawStringInst.insertChar(chr))
				rawStringBracketBuff += chr;
			else
				buff += chr;
			return true;
		}
	}

	else
	{
		if (rawStringBracket.openBracketsOperators.contains(rawStringBracketBuff))
			isCurrentRawString.emplace(rawStringBracketBuff);

		if (!isCurrentRawString.empty() && rawStringBracket.closeBracketsOperators.contains(rawStringBracketBuff) &&
			rawStringBracket.closeBracketsOperators.at(rawStringBracketBuff) == isCurrentRawString.top())
			isCurrentRawString.pop();

		buff += rawStringBracketBuff;

		startWithRawStringInst.reset();
		rawStringBracketBuff.clear();

		if (startWithRawStringInst.insertChar(chr))
			rawStringBracketBuff += chr;
		else
			buff += chr;
		return true;
	}

	return false;
}

std::vector<std::string> Lexer::lexing(const std::string& currContent) const {
	std::string buff{ "" };
	std::string rawStringBracketBuff{ "" };
	std::vector<std::string> temp;
	TrieTree::StartsWithsInstance startWithInst(mKeywordTree);
	TrieTree::StartsWithsInstance startWithRawStringInst(mRawStringBracketTree);

	std::stack<std::string> isCurrentRawString;

	buff.reserve(50);
	temp.reserve(50);

	auto clearBuffer = [&buff, &temp, &startWithInst, this]() {
		if (mKeywordTree.search(buff))
			temp.push_back(buff);
		startWithInst.reset();
		buff.clear();
		};

	for (char chr : currContent) {
		if ((!temp.empty() &&
			(mRawStringBracket.openBracketsOperators.contains(temp.back()) || !isCurrentRawString.empty())) &&
			handleRawString(chr, temp, buff, rawStringBracketBuff, startWithInst, startWithRawStringInst, isCurrentRawString, mRawStringBracket))
			continue;

		if (std::isdigit(chr) && !startWithInst.previewInsertChar(chr)) {
			if (!buff.empty() && !std::isdigit(buff.at(0)))
				clearBuffer();
			buff += chr;
			continue;
		}

		if (!buff.empty() && std::isdigit(buff.at(0))) {
			temp.push_back(buff);
			buff.clear();
		}

		if (mSeparatorKeys.contains(chr) || !startWithInst.insertChar(chr)) {
			clearBuffer();
			if (startWithInst.insertChar(chr) || 
				(!temp.empty() && mRawStringBracket.openBracketsOperators.contains(temp.back())))
				buff += chr;
			continue;
		}

		buff += chr;
	}

	if (!isCurrentRawString.empty() && mRawStringBracket.closeBracketsOperators.contains(rawStringBracketBuff))
	{
		nonEmptyPushback(temp, buff);
		nonEmptyPushback(temp, rawStringBracketBuff);
	}
	else if (!isCurrentRawString.empty())
		nonEmptyPushback(temp, buff + rawStringBracketBuff);

	else if (buff.length() && (mKeywordTree.search(buff) || std::isdigit(buff.at(0))))
		temp.push_back(buff);

	return temp;
}

std::vector<std::string> Lexer::lexing(
	const TrieTree& keywordTree,
	const TrieTree& rawStringBracketTree,
	const std::unordered_set<char>& separatorKeys,
	const Brackets& rawStringBracket,
	const std::string& currContent) {

	std::string buff{ "" };
	std::string rawStringBracketBuff{ "" };
	std::vector<std::string> temp;
	TrieTree::StartsWithsInstance startWithInst(keywordTree);
	TrieTree::StartsWithsInstance startWithRawStringInst(rawStringBracketTree);

	std::stack<std::string> isCurrentRawString;

	buff.reserve(50);
	temp.reserve(50);

	auto clearBuffer = [&buff, &temp, &startWithInst, &keywordTree]() {
		if (keywordTree.search(buff))
			temp.push_back(buff);
		startWithInst.reset();
		buff.clear();
		};

	for (char chr : currContent) {
		if ((!temp.empty() &&
			(rawStringBracket.openBracketsOperators.contains(temp.back()) || !isCurrentRawString.empty())) &&
			handleRawString(chr, temp, buff, rawStringBracketBuff, startWithInst, startWithRawStringInst, isCurrentRawString, rawStringBracket))
			continue;

		if (std::isdigit(chr) && !startWithInst.previewInsertChar(chr)) {
			if (!buff.empty() && !std::isdigit(buff.at(0)))
				clearBuffer();
			buff += chr;
			continue;
		}

		if (!buff.empty() && std::isdigit(buff.at(0))) {
			temp.push_back(buff);
			buff.clear();
		}

		if (separatorKeys.contains(chr) || !startWithInst.insertChar(chr)) {
			clearBuffer();
			if (startWithInst.insertChar(chr) || (!temp.empty() && rawStringBracket.openBracketsOperators.contains(temp.back())))
				buff += chr;
			continue;
		}

		buff += chr;
	}

	if (!isCurrentRawString.empty() && rawStringBracket.closeBracketsOperators.contains(rawStringBracketBuff))
	{
		nonEmptyPushback(temp, buff);
		nonEmptyPushback(temp, rawStringBracketBuff);
	}
	else if (!isCurrentRawString.empty())
		nonEmptyPushback(temp, buff + rawStringBracketBuff);

	else if (buff.length() && (keywordTree.search(buff) || (buff.length() && std::isdigit(buff.at(0)))))
		temp.push_back(buff);

	return temp;
}


void Lexer::_reinitializeKeyWordTree()
{
	for (const auto& keyword : mKeywords) {
		if (!mKeywordTree.search(keyword))
			mKeywordTree.insert(keyword);
	}
}