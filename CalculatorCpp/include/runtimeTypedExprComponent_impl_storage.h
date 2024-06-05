#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_STORAGE
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_STORAGE

#include <sstream>
#include <format>
#include "runtimeTypedExprComponent.h"

inline Storage::Storage(const RuntimeCompoundType& storageType, const StorageArguments& storageData) :
	BaseRuntimeTypedExprComponent(storageType, NodeFactory::NodePosNull),
	mStorageInfo{ RuntimeCompoundType::getStorageInfo(storageType) },
	mStroageData{ storageData } {}

inline Storage::Storage(RuntimeCompoundType&& storageType, StorageArguments&& storageData) :
	BaseRuntimeTypedExprComponent(std::move(storageType), NodeFactory::NodePosNull),
	mStorageInfo{ RuntimeCompoundType::getStorageInfo(storageType) },
	mStroageData{ std::move(storageData) } {}

inline Storage::Storage(const Storage& other) :
	BaseRuntimeTypedExprComponent(other.getType(), other._getNodeExpression()),
	mStorageInfo{ other.mStorageInfo },
	mStroageData{ other.mStroageData } {}

inline Storage::Storage(Storage&& other) noexcept :
	BaseRuntimeTypedExprComponent(std::move(other.mType), other._getNodeExpression()),
	mStorageInfo{ std::move(other.mStorageInfo) },
	mStroageData{ std::move(other.mStroageData) } {}

inline Storage& Storage::operator=(const Storage& other) {
	if (this != &other) {
		mType = other.getType();
		mNodeExpression = other._getNodeExpression();
		mStroageData = other.mStroageData;
		mStorageInfo = other.mStorageInfo;
	}

	return *this;
}

inline Storage& Storage::operator=(Storage&& other) noexcept {
	if (this != &other) {
		mType = std::move(other.mType);
		mNodeExpression = other._getNodeExpression();
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

	if (RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(std::move(storageDataTypes)) != storageType)
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

	return Storage(RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(storageDataTypes), storageData);
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

	return Storage(RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(std::move(storageDataTypes)), std::move(storageData));
}

inline Storage Storage::NullStorage() {
	return Storage(RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage({}), {});
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
	if (!NodeFactory::validNode(storageRootNode) || NodeFactory::node(storageRootNode).nodeState != NodeFactory::Node::NodeState::Storage)
		return std::runtime_error("storageRootNode must be valid node with Storage nodestate.");

	Result<RuntimeType, std::runtime_error> storageTypeRaw = getReturnType(storageRootNode, EvaluatorLambdaFunctions, &NodeFactory::getNodesCachedType());
	EXCEPT_RETURN(storageTypeRaw);

	if (storageTypeRaw.getValue() == RuntimeBaseType::_Storage)
		return Storage::NullStorage();

	std::vector<NodeFactory::NodePos> argumentNodeExpressions;
	NodeFactory::NodePos currArgNodePos = storageRootNode;
	while (NodeFactory::validNode(currArgNodePos)) {
		argumentNodeExpressions.emplace_back(NodeFactory::node(currArgNodePos).leftPos);
		currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
	}

	Result<StorageArguments, std::runtime_error> argumentsResult{ Lambda::_NodeExpressionsEvaluator(argumentNodeExpressions, EvaluatorLambdaFunctions) };

	if (argumentsResult.isError())
		return RuntimeError<StorageEvaluationError>(
			argumentsResult.getException(),
			"When trying to evalute arguments of Storage",
			"Storage::fromExpressionNode"
		);

	if (!argumentsResult.getValue().size())
		return NullStorage();

	return Storage(std::get<RuntimeCompoundType>(storageTypeRaw.getValue()), argumentsResult.moveValue());
}

/**
 * @throws std::runtime_error If an index out of range.
 */
inline const RuntimeTypedExprComponent& Storage::operator[](size_t index) const {
	if (index >= mStroageData.size())
		throw std::runtime_error(std::format("Cannot access out of range index \"{}\". (from Storage::operator[])", index));
	return mStroageData[index];
}

inline const std::vector<RuntimeTypedExprComponent>& Storage::getData() const {
	return mStroageData;
}

inline size_t Storage::size() const {
	return mStroageData.size();
}

inline NodeFactory::NodePos Storage::generateExpressionTree() const {
	NodePos root = NodeFactory::create();
	NodePos tail = root;

	NodeFactory::node(root).nodeState = NodeFactory::Node::NodeState::Storage;

	if (mStroageData.empty())
		return root;

	NodeFactory::node(tail).leftPos = mStroageData.front().toNodeExpression();

	for (size_t i{ 1 }; i < mStroageData.size(); i++) {
		NodePos curr = NodeFactory::create();
		NodeFactory::node(curr).leftPos = mStroageData[i].toNodeExpression();
		NodeFactory::node(tail).rightPos = curr;
		tail = curr;
	}

	return root;
}

inline NodeFactory::NodePos Storage::storageLikeIteratorNext(NodePos currNodePos) {
	if (!NodeFactory::validNode(currNodePos))
		return NodeFactory::NodePosNull;
	return NodeFactory::node(currNodePos).rightPos;
}

inline NodeFactory::NodePos Storage::storageLikeIteratorEnd(NodePos currNodePos) {
	if (!NodeFactory::validNode(currNodePos))
		return NodeFactory::NodePosNull;
	while (NodeFactory::node(currNodePos).rightPos != NodeFactory::NodePosNull)
		currNodePos = NodeFactory::node(currNodePos).rightPos;
	return currNodePos;
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