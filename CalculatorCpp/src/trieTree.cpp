#include "trieTree.h"

void TrieTree::insert(const std::string& word) {
	TrieNode* currNode = root.get();
	for (char chr : word) {
		if (!currNode->children.contains(chr)) {
			currNode->children[chr] = std::make_unique<TrieNode>();
		}
		currNode = currNode->children[chr].get();
	}
	currNode->isEndOfWord = true;
	validWords.insert(word);
}

bool TrieTree::search(const std::string& word) const {
	return validWords.contains(word);
}

bool TrieTree::startsWith(const std::string& prefix) const {
	TrieNode* currNode = root.get();
	for (char chr : prefix) {
		if (currNode->children.contains(chr)) {
			return false;
		}
		currNode = currNode->children[chr].get();
	}
	return true;
}

bool TrieTree::remove(const std::string& word) {
	TrieNode* currNode = root.get();
	std::vector<std::pair<TrieNode*, char>> path;

	for (char chr : word) {
		if (!currNode->children.contains(chr)) 
			return false;
		path.emplace_back(currNode, chr);
		currNode = currNode->children[chr].get();
	}

	if (!currNode->isEndOfWord)
		return false;
	

	currNode->isEndOfWord = false;
	validWords.erase(word);

	for (auto it = path.rbegin(); it != path.rend(); ++it) {
		TrieNode* node = it->first;
		char chr = it->second;
		if (node->children[chr]->children.empty() && !node->children[chr]->isEndOfWord) 
			node->children.erase(chr);
		else 
			break;
	}
	return true;
}


TrieTree::StartsWithsInstance::StartsWithsInstance(const TrieTree& trieTree) : mRootNode{ trieTree.root.get() }, mCurrentTrieNode{ trieTree.root.get() } {}

bool TrieTree::StartsWithsInstance::insertChar(const char currChar) {
	if (!mCurrentTrieNode->children.contains(currChar)) {
		mResult = false;
		return false;
	}
	mCurrentTrieNode = mCurrentTrieNode->children[currChar].get();
	mResult = true;
	return true;
}

bool TrieTree::StartsWithsInstance::previewInsertChar(const char currChar) {
	if (!mCurrentTrieNode->children.contains(currChar)) {
		mResult = false;
		return false;
	}
	return true;
}

bool TrieTree::StartsWithsInstance::getResult() const {
	return mResult;
}

void TrieTree::StartsWithsInstance::reset() {
	mCurrentTrieNode = mRootNode;
}