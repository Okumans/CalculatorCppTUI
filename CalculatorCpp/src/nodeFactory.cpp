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
	return static_cast<NodePos>(mNodeData.size()) - 1;
}

NodeFactory::Node& NodeFactory::iNode(NodePos index) {
	return mNodeData[index];
}

bool NodeFactory::iValidNode(NodePos index) const {
	return (index < mNodeData.size());
}

NodeFactory::Node& NodeFactory::node(NodePos index) {
	return iGetInstance().iNode(index);
}
NodeFactory::NodePos NodeFactory::create(const std::string& value) {
	return iGetInstance().iCreate(value);
}

bool NodeFactory::validNode(NodePos index) {
	return iGetInstance().iValidNode(index);
}

void NodeFactory::reserve(size_t amount) {
	iGetInstance().mNodeData.reserve(amount);
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

bool NodeFactory::Node::operator==(const Node& other) const {
	return (value == other.value && nodestate == other.nodestate && utilityStorage == other.utilityStorage);
}


