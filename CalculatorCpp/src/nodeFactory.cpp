#include "nodeFactory.h"

NodeFactory& NodeFactory::iGetInstance() {
	static NodeFactory instance;
	return instance;
}

void NodeFactory::iFreeAll() {
	mNodeData.clear();
}

NodeFactory::NodePos NodeFactory::iCreate(const std::string& value) {
	mNodeData.emplace_back(value);
	return mNodeData.size() - 1;
}

NodeFactory::Node& NodeFactory::iNode(NodePos index) {
	return mNodeData[index];
}

NodeFactory::Node& NodeFactory::node(NodePos index) {
	return iGetInstance().iNode(index);
}
NodeFactory::NodePos NodeFactory::create(const std::string& value) {
	return iGetInstance().iCreate(value);
}

void NodeFactory::freeAll() {
	iGetInstance().iFreeAll();
}

NodeFactory::Node::Node(const std::string& value) : value{ value } {}

NodeFactory::Node& NodeFactory::Node::rightNode() {
	return NodeFactory::iGetInstance().iNode(rightPos);
}

NodeFactory::Node& NodeFactory::Node::leftNode() {
	return NodeFactory::iGetInstance().iNode(leftPos);
}

