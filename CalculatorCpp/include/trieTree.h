#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>

class TrieTree {
public:
    class TrieNode {
    public:
        bool isEndOfWord = false;
        std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    };

private:
    std::unique_ptr<TrieNode> root = std::make_unique<TrieNode>();
    std::unordered_set<std::string> validWords;

public:
    void insert(const std::string& word);

    bool search(const std::string& word) const;

    bool startsWith(const std::string& prefix) const;

    class StartsWithsInstance {
    private:
        TrieNode * mRootNode;
        TrieNode* mCurrentTrieNode;
        bool mResult{ false };
    public:
        explicit StartsWithsInstance(const TrieTree &trieTree);
        bool insertChar(const char currChar);
        bool previewInsertChar(const char currChar);
        bool getResult() const;
        void reset();
    };
};
