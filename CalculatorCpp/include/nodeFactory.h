#pragma once

#include <vector>
#include <string>
#include <limits>
#include <tuple>
#include <cstdint>

#include "runtimeType.h"

class NodeFactory {
public:
	using NodePos = uint32_t;
	static const NodePos NodePosNull = std::numeric_limits<NodePos>::max();

	class Node
	{
	public:
		enum class NodeState : int8_t {
			Number,
			LambdaFuntion,
			Storage,
			Operator,
		};

		std::string value;
		NodeState nodeState{ NodeState::Number };
		std::vector<std::pair<std::string, RuntimeType>> parametersWithType;

		NodePos leftPos{ std::numeric_limits<NodePos>::max() };
		NodePos rightPos{ std::numeric_limits<NodePos>::max() };

		explicit Node(const std::string& value);
		Node& rightNode();
		Node& leftNode();

		bool operator==(const Node& other) const;
	};

private:
	NodeFactory() = default;
	std::vector<Node> mNodeData;
	std::unordered_map<NodePos, RuntimeType> mNodesCachedType;

public:
	NodeFactory(const NodeFactory& other) = delete;
	NodeFactory& operator=(const NodeFactory& other) = delete;
	static NodeFactory& iGetInstance();
	static Node& node(NodePos index);
	static NodePos create(const std::string& value = "");
	static void freeAll();
	static bool validNode(NodePos index);
	static void reserve(size_t amount);
	static size_t size();
	static std::unordered_map<NodePos, RuntimeType>& getNodesCachedType();

private:
	Node& iNode(NodePos index);
	NodePos iCreate(const std::string& value);
	bool iValidNode(NodePos index) const;
	void iFreeAll();
};
