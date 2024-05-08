#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_STORAGE
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_STORAGE

#include <sstream>
#include <format>
#include "runtimeTypedExprComponent.h"

inline Storage::Storage(const RuntimeCompoundType& storageType, const StorageArguments& storageData) :
	BaseRuntimeTypedExprComponent(storageType, generateExpressionTree(storageData)),
	mStorageInfo{ RuntimeCompoundType::getStorageInfo(storageType) },
	mStroageData{ storageData } {}

inline Storage::Storage(RuntimeCompoundType&& storageType, StorageArguments&& storageData) :
	BaseRuntimeTypedExprComponent(std::move(storageType), generateExpressionTree(storageData)),
	mStorageInfo{ RuntimeCompoundType::getStorageInfo(storageType) },
	mStroageData{ std::move(storageData) } {}

inline Storage::Storage(const Storage& other) :
	BaseRuntimeTypedExprComponent(other.mType, other.mNodeExpression),
	mStorageInfo{ other.mStorageInfo },
	mStroageData{ other.mStroageData } {}

inline Storage::Storage(Storage&& other) noexcept :
	BaseRuntimeTypedExprComponent(std::move(other.mType), other.mNodeExpression),
	mStorageInfo{ std::move(other.mStorageInfo) },
	mStroageData{ std::move(other.mStroageData) } {}

inline Storage& Storage::operator=(const Storage& other) {
	if (this != &other) {
		mType = other.mType;
		mNodeExpression = other.mNodeExpression;
		mStroageData = other.mStroageData;
		mStorageInfo = other.mStorageInfo;
	}

	return *this;
}

inline Storage& Storage::operator=(Storage&& other) noexcept {
	if (this != &other) {
		mType = std::move(other.mType);
		mNodeExpression = other.mNodeExpression;
		mStroageData = std::move(other.mStroageData);
		mStorageInfo = std::move(other.mStorageInfo);
	}

	return *this;
}

inline Result<Storage, std::runtime_error> Storage::fromVector(const RuntimeCompoundType& storageType, const StorageArguments& storageData) {
	std::vector<RuntimeType> storageDataTypes;
	storageDataTypes.reserve(storageData.size());

	for (const auto& storageArg : storageData) {
		std::visit([&storageDataTypes](const auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Number> ||
				std::is_same_v<T, Storage> ||
				std::is_same_v<T, Lambda>) {
				storageDataTypes.emplace_back(arg.getType());
			}
			}, storageArg);
	}

	if (RuntimeCompoundType::Storage(std::move(storageDataTypes)) != storageType)
		return std::runtime_error("Storage argument type and storage configured type not matched.");

	return Storage(storageType, storageData);
}

inline Storage Storage::fromVector(const StorageArguments& storageData) {
	std::vector<RuntimeType> storageDataTypes;
	storageDataTypes.reserve(storageData.size());

	for (const auto& storageArg : storageData) {
		std::visit([&storageDataTypes](const auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Number> ||
				std::is_same_v<T, Storage> ||
				std::is_same_v<T, Lambda>) {
				storageDataTypes.emplace_back(arg.getType());
			}
			}, storageArg);
	}

	return Storage(RuntimeCompoundType::Storage(storageDataTypes), storageData);
}

inline Storage Storage::fromVector(StorageArguments&& storageData) {
	std::vector<RuntimeType> storageDataTypes;
	storageDataTypes.reserve(storageData.size());

	for (const auto& storageArg : storageData) {
		std::visit([&storageDataTypes](const auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Number> ||
				std::is_same_v<T, Storage> ||
				std::is_same_v<T, Lambda>) {
				storageDataTypes.emplace_back(arg.getType());
			}
			}, storageArg);
	}

	return Storage(RuntimeCompoundType::Storage(std::move(storageDataTypes)), std::move(storageData));
}

inline Storage Storage::NullStorage() {
	return Storage(RuntimeCompoundType::Storage({}), {});
}

template <RuntimeTypedExprComponentRequired ...Args>
inline Storage Storage::fromArgs(Args &&...storageData) {
	constexpr size_t count = sizeof...(storageData);
	std::vector<RuntimeType> storageDataTypes;
	std::vector<RuntimeTypedExprComponent> tmp;
	storageDataTypes.reserve(count);
	tmp.reserve(count);

	(tmp.emplace_back(std::move(std::forward<Args>(storageData))), ...);

	for (const auto& storageArg : tmp) {
		std::visit([&storageDataTypes](const auto& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Number> ||
				std::is_same_v<T, Storage> ||
				std::is_same_v<T, Lambda>) {
				storageDataTypes.emplace_back(arg.getType());
			}
			}, storageArg);
	}

	return Storage(RuntimeCompoundType::Storage(std::move(storageDataTypes)), std::move(tmp));
}

inline Result<Storage, std::runtime_error> Storage::fromExpressionNode(NodePos storageRootNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions) {
	if (!NodeFactory::validNode(storageRootNode) || NodeFactory::node(storageRootNode).nodestate != NodeFactory::Node::NodeState::Storage)
		return std::runtime_error("storageRootNode must be valid node with Storage nodestate.");

	Result<RuntimeType, std::runtime_error> storageTypeRaw = getReturnType(storageRootNode, EvaluatorLambdaFunctions);
	EXCEPT_RETURN(storageTypeRaw);

	StorageArguments arguments;
	NodeFactory::NodePos currArgNodePos = storageRootNode;
	while (NodeFactory::validNode(currArgNodePos)) {
		NodeFactory::Node::NodeState currNodeState = NodeFactory::node(currArgNodePos).leftNode().nodestate;
		NodePos currArgNodeLeftPos = NodeFactory::node(currArgNodePos).leftPos;
		switch (currNodeState)
		{
		case NodeFactory::Node::NodeState::LambdaFuntion:
		{
			Result result = Lambda::fromExpressionNode(currArgNodeLeftPos, EvaluatorLambdaFunctions);
			EXCEPT_RETURN(result);
			arguments.emplace_back(result.getValue());
			break;
		}
		case NodeFactory::Node::NodeState::Operator:
		{
			Result result = Lambda::fromExpressionNode(currArgNodeLeftPos, EvaluatorLambdaFunctions);
			EXCEPT_RETURN(result);
			arguments.emplace_back(result.getValue());
			break;
		}
		case NodeFactory::Node::NodeState::Number:
		{
			Number result = Number::fromExpressionNode(currArgNodeLeftPos);
			arguments.emplace_back(result);
			break;
		}
		case NodeFactory::Node::NodeState::Storage:
		{
			Result result = Storage::fromExpressionNode(currArgNodeLeftPos, EvaluatorLambdaFunctions);
			EXCEPT_RETURN(result);
			arguments.emplace_back(result.getValue());
			break;
		}
		}
		currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
	}

	if (!arguments.size())
		return std::runtime_error("Cannot evalutate noting.");

	return Storage(std::get<RuntimeCompoundType>(storageTypeRaw.getValue()), arguments);
}

/**
 * @throws std::runtime_error If an index out of range.
 */
inline const RuntimeTypedExprComponent& Storage::operator[](size_t index) const {
	if (index >= mStroageData.size())
		throw std::runtime_error(std::format("Cannot access out of range index \"{}\". (from Storage::operator[])", index));
	return mStroageData[index];
}

inline size_t Storage::size() const {
	return mStroageData.size();
}

inline NodeFactory::NodePos Storage::generateExpressionTree(const StorageArguments& storageData) const {
	NodePos root = NodeFactory::create();
	NodePos tail = root;

	NodeFactory::node(root).nodestate = NodeFactory::Node::NodeState::Storage;

	if (storageData.empty())
		return root;

	NodeFactory::node(tail).leftPos = storageData.front().toNodeExpression();

	for (size_t i{ 1 }; i < storageData.size(); i++)
	{
		NodePos curr = NodeFactory::create();
		NodeFactory::node(curr).leftPos = storageData[i].toNodeExpression();
		NodeFactory::node(tail).rightPos = curr;
		tail = curr;
	}

	return root;
}

inline std::string Storage::toString() const {
	std::stringstream ss;
	ss << "Storage[";
	bool first = true;
	for (const auto& element : mStroageData) {
		if (!first) {
			ss << ", ";
		}
		ss << element.toString();
		first = false;
	}
	ss << "]";

	return ss.str();
}

#endif // RUNTIME_TYPED_EXPR_COMPONENT_IMPL_STORAGE