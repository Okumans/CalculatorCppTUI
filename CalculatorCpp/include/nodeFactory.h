#pragma once

#include <vector>
#include <string>

class NodeFactory {
public:
	using NodePos = size_t;
	class Node
	{
	public:
		enum class NodeState {
			None,
			LambdaFuntion,
			Storage
		};

		std::string value;
		NodeState nodestate{ NodeState::None };
		std::vector<std::string> utilityStorage;

		NodePos leftPos{ 0 };
		NodePos rightPos{ 0 };

		explicit Node(const std::string& value);
		Node& rightNode();
		Node& leftNode();
	};

private:
	NodeFactory() = default;
	std::vector<Node> mNodeData;

public:
	NodeFactory(const NodeFactory& other) = delete;
	NodeFactory& operator=(const NodeFactory& other) = delete;
	static NodeFactory& iGetInstance();
	static Node& node(NodePos index);
	static NodePos create(const std::string& value);
	static void freeAll();


private:
	Node& iNode(NodePos index);
	NodePos iCreate(const std::string& value);
	void iFreeAll();
};