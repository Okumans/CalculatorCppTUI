#include "lexer.h"
#include <cstddef>
#include <vector>
#include <iterator>
#include <cctype>

#define DEBUG
#include "debug.cpp"

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
	reinitializeKeyWordTree();
}

void Lexer::setSeperatorKeys(const std::unordered_set<char>& keys) {
	mSeparatorKeys = keys;
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
	std::vector<std::string> temp;
	TrieTree::StartsWithsInstance startWithInst(mKeywordTree);

	buff.reserve(50);
	temp.reserve(50);

	auto clearBuffer = [&buff, &temp, &startWithInst, this]() {
		if (mKeywordTree.search(buff))
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

		if (mSeparatorKeys.contains(chr) || !startWithInst.insertChar(chr)) {
			clearBuffer();
			if (startWithInst.insertChar(chr))
				buff += chr;
			continue;
		}

		buff += chr;
	}

	if (!buff.empty() && (mKeywordTree.search(buff)) || std::isdigit(buff.at(0)))
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

void Lexer::reinitializeKeyWordTree()
{
	for (const auto& keyword : mKeywords) {
		if (!mKeywordTree.search(keyword))
			mKeywordTree.insert(keyword);
	}
}