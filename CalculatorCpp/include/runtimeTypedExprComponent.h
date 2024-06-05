#pragma once

#include <string>
#include <optional>
#include <functional>

#include "runtimeType.h"
#include "nodeFactory.h"
#include "result.h"

class Storage;
class Lambda;
class Number;
class RuntimeTypedExprComponent;

template <typename T>
concept RuntimeTypedExprComponentRequired = std::is_convertible_v<T, RuntimeTypedExprComponent>;

struct LambdaConstructionError {
	static const std::string prefix;
};
inline const std::string LambdaConstructionError::prefix = "LambdaConstructionError";

struct LambdaEvaluationError {
	static const std::string prefix;
};
inline const std::string LambdaEvaluationError::prefix = "LambdaEvaluationError";

struct StorageEvaluationError {
	static const std::string prefix;
};
inline const std::string StorageEvaluationError::prefix = "StorageEvaluationError";

class BaseRuntimeTypedExprComponent {
protected:
	using NodePos = NodeFactory::NodePos;

public:
	virtual std::string toString() const = 0;
	virtual NodePos generateExpressionTree() const = 0;

	const RuntimeType& getType() const {
		return mType;
	}

	NodePos getNodeExpression() const {
		if (!NodeFactory::validNode(mNodeExpression))
			_setNodeExpression(generateExpressionTree());
		return mNodeExpression;
	}

protected:
	RuntimeType mType;
	mutable NodePos mNodeExpression{ NodeFactory::NodePosNull };

	BaseRuntimeTypedExprComponent(const RuntimeType& type, NodePos nodeExpression) :
		mType{ type },
		mNodeExpression{ nodeExpression } {}

	BaseRuntimeTypedExprComponent(RuntimeType&& type, NodePos nodeExpression) :
		mType{ std::move(type) },
		mNodeExpression{ nodeExpression } {}

	void _setNodeExpression(NodePos nodeExpression) const {
		mNodeExpression = nodeExpression;
	}

	NodePos _getNodeExpression() const {
		return mNodeExpression;
	}

	friend std::ostream& operator<<(std::ostream& os, BaseRuntimeTypedExprComponent const& brttexc) {
		os << brttexc.toString();
		return os;
	}
};

std::vector<std::string_view> splitString(std::string_view in, char sep);
bool _fastCheckRuntimeTypeArgumentsType(const RuntimeType& baseType, const std::vector<RuntimeTypedExprComponent>& argumentsCheckType);
Result<RuntimeType, std::runtime_error> getReturnType(NodeFactory::NodePos rootExpressionNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, std::unordered_map<NodeFactory::NodePos, RuntimeType>* nodesTypeCache = nullptr);

class Number : public BaseRuntimeTypedExprComponent {
public:
	Number(long double number);
	Number();
	static Number fromExpressionNode(NodePos numberNodeExpression);
	NodePos generateExpressionTree() const override;

	long double getNumber() const;
	std::string toString() const override;

	operator long double() const;

private:
	long double mNumber;
	Number(NodePos numberExpression, bool);
};

class Lambda : public BaseRuntimeTypedExprComponent {
public:
	using LambdaArguments = std::vector<RuntimeTypedExprComponent>;
	enum class LambdaNotation {
		Infix,
		Postfix,
		Prefix,
		Constant
	};

	Lambda(const Lambda& other);
	Lambda& operator=(const Lambda& other);
	Lambda(Lambda&& other) noexcept;
	Lambda& operator=(Lambda&& other) noexcept;

	static Result<Lambda, std::runtime_error> fromFunction(const std::string& lambdaFunctionSignature, const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction);
	static Result<Lambda, std::runtime_error> fromFunction(const std::string& lambdaFunctionSignature, const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction, const LambdaArguments& testArgument);
	static Result<Lambda, std::runtime_error> fromExpressionNode(NodePos lambdaFunctionRootNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	static Lambda LambdaConstant(const std::string& functionSignature, const RuntimeTypedExprComponent& constValue);
	static Lambda LambdaConstant(std::string&& functionSignature, RuntimeTypedExprComponent&& constValue);

	template<RuntimeTypedExprComponentRequired ...Args>
	Result<RuntimeTypedExprComponent, std::runtime_error> evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, Args&&... arguments) const;
	Result<RuntimeTypedExprComponent, std::runtime_error> evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const LambdaArguments& arguments) const;
	Result<NodePos, std::runtime_error> getExpressionTree(const LambdaArguments& arguments) const;
	RuntimeCompoundType::LambdaInfo getLambdaInfo() const;
	std::optional<std::string_view> getLambdaSignature() const;
	LambdaNotation getNotation() const;
	std::string toString() const override;
	NodePos generateExpressionTree(const std::string& functionSignature) const;
	NodePos generateExpressionTree() const override;

private:
	RuntimeCompoundType::LambdaInfo mLambdaInfo;
	LambdaNotation mLambdaNotation;

	std::variant<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>, NodePos> mLambdaFunction;
	std::optional<std::string> mLambdaFunctionSignature;

	Lambda(const std::string& lambdaFunctionSignature, const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction);
	Lambda(std::string&& lambdaFunctionSignature, RuntimeCompoundType&& lambdaType, LambdaNotation lambdaNotation, std::function<RuntimeTypedExprComponent(LambdaArguments)>&& lambdaFunction);
	Lambda(const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, NodePos lambdaFunctionRootNode);
	static Result<RuntimeTypedExprComponent, std::runtime_error> _NodeExpressionEvaluate(NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	static Result<std::vector<RuntimeTypedExprComponent>, std::runtime_error> _NodeExpressionsEvaluator(std::vector<NodePos> rootNodeExpressions, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	static void findAndReplaceConstant(NodeFactory::NodePos root, const std::unordered_map<std::string, NodeFactory::NodePos>& replacement);

	friend class Storage;
	friend class Evaluate;
	friend class Parser;
};

class Storage : public BaseRuntimeTypedExprComponent {
public:
	using StorageArguments = std::vector<RuntimeTypedExprComponent>;

	Storage(const Storage& other);
	Storage(Storage&& other) noexcept;
	Storage& operator=(const Storage& other);
	Storage& operator=(Storage&& other) noexcept;

	static Storage NullStorage();
	static Storage fromVector(const StorageArguments& storageData);
	static Storage fromVector(StorageArguments&& storageData);
	template <RuntimeTypedExprComponentRequired ...Args>
	static Storage fromArgs(Args &&...storageData);
	static Result<Storage, std::runtime_error> fromVector(const RuntimeCompoundType& storageType, const StorageArguments& storageData);
	static Result<Storage, std::runtime_error> fromExpressionNode(NodePos storageRootNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	const RuntimeTypedExprComponent& operator[](size_t index) const;
	const std::vector<RuntimeTypedExprComponent>& getData() const;

	size_t size() const;
	std::string toString() const override;
	NodePos generateExpressionTree() const override;

	static NodePos storageLikeIteratorNext(NodePos currNodePos); // get next storage iterator, the value of storage like iterator will be on the left Node
	static NodePos storageLikeIteratorEnd(NodePos currNodePos); // get end storage iterator, the value of storage like iterator will be on the left Node

private:
	RuntimeCompoundType::StorageInfo mStorageInfo;
	StorageArguments mStroageData;

	Storage(const RuntimeCompoundType& storageType, const StorageArguments& storageData);
	Storage(RuntimeCompoundType&& storageType, StorageArguments&& storageData);
};

// act like a void pointer
class NodePointer : public BaseRuntimeTypedExprComponent {
public:
	// constructors
	explicit NodePointer(NodePos target);
	explicit NodePointer();

	// getter
	bool isTypeValid(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunction) const;
	bool isNodePointerValid() const;
	Result<RuntimeTypedExprComponent, std::runtime_error> getPointed(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunction) const;
	NodePos getPointerIndex() const;
	const NodeFactory::Node& getPointerNode() const;

	// setter
	void changePoint(NodePos target);

	// BaseRuntimeTypedExprComponents general feature implementors
	std::string toString() const override;
	NodePos generateExpressionTree() const override;
};

class RuntimeTypedExprComponent : public std::variant<Number, Storage, Lambda, NodePointer> {
public:
	// constructors
	RuntimeTypedExprComponent(const Storage& component);
	RuntimeTypedExprComponent(Storage&& component);
	RuntimeTypedExprComponent(const Lambda& component);
	RuntimeTypedExprComponent(Lambda&& component);
	RuntimeTypedExprComponent(const Number& component);
	RuntimeTypedExprComponent(Number&& component);
	RuntimeTypedExprComponent(const NodePointer& component);
	RuntimeTypedExprComponent(NodePointer&& component);
	RuntimeTypedExprComponent(long double component);
	RuntimeTypedExprComponent(const RuntimeTypedExprComponent& component);
	RuntimeTypedExprComponent(RuntimeTypedExprComponent&& component) noexcept;

	// static constructor
	static Result<RuntimeTypedExprComponent, std::runtime_error> fromNodeExpression(NodeFactory::NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);

	// getters
	const Number& getNumber() const;
	const Lambda& getLambda() const;
	const Storage& getStorage() const;
	const NodePointer& getNodePointer() const;
	RuntimeBaseType getTypeHolded() const;
	RuntimeType getDetailTypeHold() const;

	// operator getter
	const RuntimeTypedExprComponent& operator[](size_t index) const;

	// BaseRuntimeTypedExprComponents general feature implementors
	std::string toString() const;
	NodeFactory::NodePos toNodeExpression() const;

	// equality operators
	RuntimeTypedExprComponent& operator=(const RuntimeTypedExprComponent& other);
	RuntimeTypedExprComponent& operator=(RuntimeTypedExprComponent&& other) noexcept;

	// ostream operator
	friend std::ostream& operator<<(std::ostream& os, const RuntimeTypedExprComponent& rttexcp);
private:
	RuntimeBaseType mStoredType;
};

template <>
struct std::formatter<RuntimeTypedExprComponent> : std::formatter<std::string> {
	auto format(const RuntimeTypedExprComponent& rtt, format_context& ctx) const {
		std::ostringstream ss;
		ss << rtt;
		return formatter<string>::format(
			std::format("{}", ss.str()), ctx);
	}
};

#include "runtimeTypedExprComponent_impl_lambda.h"
#include "runtimeTypedExprComponent_impl_number.h"
#include "runtimeTypedExprComponent_impl_storage.h"
#include "runtimeTypedExprComponent_impl_nodePointer.h"
#include "runtimeTypeExprComponent_impl_utility.h"