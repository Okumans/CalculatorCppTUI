#include "lexer.h"
#include <cstddef>
#include <vector>
#include <iterator>
#include <cctype>

#define DEBUG
#include "debug.cpp"

static void nonEmptyPushback(std::vector<std::string>& vec, const std::string& str)
{
	if (!str.empty()) vec.push_back(str);
}

Lexer::Lexer() : mRawContent{ "" } {}

Lexer::Lexer(const std::string& rawContent) : mRawContent{ rawContent } {}

void Lexer::addContent(const std::string& content)
{
	mRawContent += content;
	const std::vector<std::string> result = lexing(content);
	mContent.insert(mContent.end(), result.begin(), result.end());
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

const std::vector<std::string>& Lexer::getContent() const {
	return mContent;
}

const std::string& Lexer::getRawContent() const {
	return mRawContent;
}

const TrieTree& Lexer::getKeywordTree() const {
	return mKeywordTree;
}

void Lexer::reLexing() {
	const std::vector<std::string> result = lexing(mRawContent);
	mContent = result;
}

std::vector<std::string> Lexer::lexing(const std::string& currContent) const {
	std::string buff{ "" };
	std::string rawStringBracketBuff{ "" };
	std::vector<std::string> temp;
	TrieTree::StartsWithsInstance startWithInst(mKeywordTree);
	TrieTree::StartsWithsInstance startWithRawStringInst(mRawStringBracketTree);

	size_t isCurrentRawString{ 0 };

	buff.reserve(50);
	temp.reserve(50);

	auto clearBuffer = [&buff, &temp, &startWithInst, this]() {
		if (mKeywordTree.search(buff))
			temp.push_back(buff);
		startWithInst.reset();
		buff.clear();
		};

	for (char chr : currContent) {
		if (!temp.empty() && (mRawStringBracket.openBracketsOperators.contains(temp.back()) || isCurrentRawString)) {
			if (!buff.empty() && !isCurrentRawString) {
				isCurrentRawString++;
				startWithRawStringInst.reset();
				startWithInst.reset();

				if (startWithRawStringInst.insertChar(buff.back()))
				{
					rawStringBracketBuff += buff.back();
					buff.pop_back();
				}
			}

			if (startWithRawStringInst.insertChar(chr)) {
				rawStringBracketBuff += chr;
				continue;
			}

			else if (!rawStringBracketBuff.empty() &&
				mRawStringBracket.openBracketsOperators.contains(temp.back()) &&
				mRawStringBracket.openBracketsOperators.at(temp.back()) == rawStringBracketBuff)
			{
				if (!(--isCurrentRawString))
				{
					nonEmptyPushback(temp, buff);
					nonEmptyPushback(temp, rawStringBracketBuff);
					buff.clear();
					rawStringBracketBuff.clear();
				}
				else
				{
					buff += rawStringBracketBuff;

					startWithRawStringInst.reset();
					rawStringBracketBuff.clear();

					if (startWithRawStringInst.insertChar(chr))
						rawStringBracketBuff += chr;
					else
						buff += chr;
					continue;
				}
			}

			else
			{
				if (mRawStringBracket.openBracketsOperators.contains(rawStringBracketBuff))
					isCurrentRawString++;

				buff += rawStringBracketBuff;

				startWithRawStringInst.reset();
				rawStringBracketBuff.clear();

				if (startWithRawStringInst.insertChar(chr))
					rawStringBracketBuff += chr;
				else
					buff += chr;
				continue;
			}
		}

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
			if (startWithInst.insertChar(chr) || (!temp.empty() && mRawStringBracket.openBracketsOperators.contains(temp.back())))
				buff += chr;
			continue;
		}

		buff += chr;
	}

	if (isCurrentRawString && mRawStringBracket.closeBracketsOperators.contains(rawStringBracketBuff))
	{
		nonEmptyPushback(temp, buff);
		nonEmptyPushback(temp, rawStringBracketBuff);
	}
	else if (isCurrentRawString)
		nonEmptyPushback(temp, buff + rawStringBracketBuff);

	else if (buff.length() && (mKeywordTree.search(buff) || (buff.length() && std::isdigit(buff.at(0)))))
		temp.push_back(buff);

	return temp;
}

std::vector<std::string> Lexer::lexing(const TrieTree& keywordTree, const std::unordered_set<char>& separatorKeys, const std::string& currContent) {
	std::string buff{ "" };
	std::vector<std::string> temp;
	TrieTree::StartsWithsInstance startWithInst(keywordTree);

	buff.reserve(50);
	temp.reserve(50);

	auto clearBuffer = [&buff, &temp, &startWithInst, &keywordTree]() {
		if (keywordTree.search(buff))
			temp.push_back(buff);
		startWithInst.reset();
		buff.clear();
		};

	for (char chr : currContent) {
		if (std::isdigit(chr)) {
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
			if (startWithInst.insertChar(chr))
				buff += chr;
			continue;
		}

		buff += chr;
	}

	if (!buff.empty() && (keywordTree.search(buff)) || std::isdigit(buff.at(0)))
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