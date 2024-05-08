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

// Custom exception class for runtime type errors
class LambdaConstructionError : public std::runtime_error {
public:
	// Constructor with a single message
	explicit LambdaConstructionError(const std::string& message)
		: std::runtime_error("LambdaConstructionError: " + message) {}

	// Constructor with message and origin information
	explicit LambdaConstructionError(const std::string& message, const std::string& from)
		: std::runtime_error("LambdaConstructionError: " + message + " (from: " + from + ")") {}

	// Constructor with chained error, message, and origin information
	explicit LambdaConstructionError(const std::runtime_error& baseError, const std::string& message, const std::string& from)
		: std::runtime_error("LambdaConstructionError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

// Custom exception class for runtime type errors
class LambdaEvaluationError : public std::runtime_error {
public:
	// Constructor with a single message
	explicit LambdaEvaluationError(const std::string& message)
		: std::runtime_error("LambdaEvaluationError: " + message) {}

	// Constructor with message and origin information
	explicit LambdaEvaluationError(const std::string& message, const std::string& from)
		: std::runtime_error("LambdaEvaluationError: " + message + " (from: " + from + ")") {}

	// Constructor with chained error, message, and origin information
	explicit LambdaEvaluationError(const std::runtime_error& baseError, const std::string& message, const std::string& from)
		: std::runtime_error("LambdaEvaluationError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

class StorageEvaluationError : public std::runtime_error {
public:
	// Constructor with a single message
	explicit StorageEvaluationError(const std::string& message)
		: std::runtime_error("StorageEvaluationError: " + message) {}

	// Constructor with message and origin information
	explicit StorageEvaluationError(const std::string& message, const std::string& from)
		: std::runtime_error("StorageEvaluationError: " + message + " (from: " + from + ")") {}

	// Constructor with chained error, message, and origin information
	explicit StorageEvaluationError(const std::runtime_error& baseError, const std::string& message, const std::string& from)
		: std::runtime_error("StorageEvaluationError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

class BaseRuntimeTypedExprComponent {
protected:
	using NodePos = NodeFactory::NodePos;

public:
	virtual std::string toString() const = 0;
	const RuntimeType& getType() const {
		return mType;
	}
	NodePos getNodeExpression() const {
		return mNodeExpression;
	}

protected:
	RuntimeType mType;
	NodePos mNodeExpression;

	BaseRuntimeTypedExprComponent(const RuntimeType& type, NodePos nodeExpression) :
		mType{ type },
		mNodeExpression{ nodeExpression } {}

	BaseRuntimeTypedExprComponent(RuntimeType&& type, NodePos nodeExpression) :
		mType{ std::move(type) },
		mNodeExpression{ nodeExpression } {}

	friend std::ostream& operator<<(std::ostream& os, BaseRuntimeTypedExprComponent const& brttexc) {
		os << brttexc.toString();
		return os;
	}
};

std::vector<std::string_view> splitString(std::string_view in, char sep);
bool _fastCheckRuntimeTypeArgumentsType(const RuntimeType& baseType, const std::vector<RuntimeTypedExprComponent>& argumentsCheckType);
Result<RuntimeType, std::runtime_error> getReturnType(NodeFactory::NodePos rootExpressionNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, bool useCache = true);

class Number : public BaseRuntimeTypedExprComponent {
public:
	Number(long double number);
	static Number fromExpressionNode(NodePos numberNodeExpression);

	long double getNumber() const;
	std::string toString() const;

	operator long double() const;

private:
	long double mNumber;
	Number(NodePos numberExpression, bool);
	NodePos generateExpressionTree(long double number) const;
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

	template<RuntimeTypedExprComponentRequired ...Args>
	Result<RuntimeTypedExprComponent, std::runtime_error> evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, Args&&... arguments) const;
	Result<RuntimeTypedExprComponent, std::runtime_error> evaluate(const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, const LambdaArguments& arguments) const;
	Result<NodePos, std::runtime_error> getExpressionTree(const LambdaArguments& arguments) const;
	RuntimeCompoundType::LambdaInfo getLambdaInfo() const;
	std::optional<std::string_view> getLambdaSignature() const;
	LambdaNotation getNotation() const;
	std::string toString() const;

private:
	RuntimeCompoundType::LambdaInfo mLambdaInfo;
	LambdaNotation mLambdaNotation;

	std::variant<std::shared_ptr<std::function<RuntimeTypedExprComponent(LambdaArguments)>>, NodePos> mLambdaFunction;
	std::optional<std::string> mLambdaFunctionSignature;

	Lambda(const std::string& lambdaFunctionSignature, const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, const std::function<RuntimeTypedExprComponent(LambdaArguments)>& lambdaFunction);
	Lambda(const RuntimeCompoundType& lambdaType, LambdaNotation lambdaNotation, NodePos lambdaFunctionRootNode);
	NodePos generateExpressionTree(const std::string& functionSignature) const;
	static Result<RuntimeTypedExprComponent, std::runtime_error> _NodeExpressionEvaluate(NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);

	friend class Storage;
	friend class Evaluate;
};

class Storage : public BaseRuntimeTypedExprComponent {
public:
	using StorageArguments = std::vector<RuntimeTypedExprComponent>;

	Storage(const Storage& other);
	Storage(Storage&& other) noexcept;
	Storage& operator=(const Storage& other);
	Storage& operator=(Storage&& other) noexcept;

	static Storage fromVector(const StorageArguments& storageData);
	static Storage fromVector(StorageArguments&& storageData);
	template <RuntimeTypedExprComponentRequired ...Args>
	static Storage fromArgs(Args &&...storageData);
	static Result<Storage, std::runtime_error> fromVector(const RuntimeCompoundType& storageType, const StorageArguments& storageData);
	static Result<Storage, std::runtime_error> fromExpressionNode(NodePos storageRootNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	const RuntimeTypedExprComponent& operator[](size_t index) const;
	size_t size() const;
	std::string toString() const;

private:
	RuntimeCompoundType::StorageInfo mStorageInfo;
	StorageArguments mStroageData;

	Storage(const RuntimeCompoundType& storageType, const StorageArguments& storageData);
	Storage(RuntimeCompoundType&& storageType, StorageArguments&& storageData);
	NodePos generateExpressionTree(const StorageArguments& storageData) const;
};

class RuntimeTypedExprComponent : public std::variant<Number, Storage, Lambda> {
public:
	const Number& getNumber() const;
	const Lambda& getLambda() const;
	const Storage& getStorage() const;
	RuntimeBaseType getTypeHolded() const;
	RuntimeType getDetailTypeHold() const;
	const RuntimeTypedExprComponent& operator[](size_t index) const;
	RuntimeTypedExprComponent(const Storage& component);
	RuntimeTypedExprComponent(Storage&& component);
	RuntimeTypedExprComponent(const Lambda& component);
	RuntimeTypedExprComponent(Lambda&& component);
	RuntimeTypedExprComponent(const Number& component);
	RuntimeTypedExprComponent(Number&& component);
	RuntimeTypedExprComponent(long double component);
	RuntimeTypedExprComponent(const RuntimeTypedExprComponent& component);
	RuntimeTypedExprComponent(RuntimeTypedExprComponent&& component) noexcept;
	static Result<RuntimeTypedExprComponent, std::runtime_error> fromNodeExpression(NodeFactory::NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions);
	std::string toString() const;
	NodeFactory::NodePos toNodeExpression() const;
	RuntimeTypedExprComponent& operator=(const RuntimeTypedExprComponent& other);
	RuntimeTypedExprComponent& operator=(RuntimeTypedExprComponent&& other) noexcept;
private:
	RuntimeBaseType mStoredType;

	friend std::ostream& operator<<(std::ostream& os, const RuntimeTypedExprComponent& rttexcp);
};

template <>
struct std::formatter<RuntimeTypedExprComponent> : std::formatter<std::string> {
	auto format(RuntimeTypedExprComponent rtt, format_context& ctx) const {
		std::ostringstream ss;
		ss << rtt;
		return formatter<string>::format(
			std::format("{}", ss.str()), ctx);
	}
};

#include "runtimeTypedExprComponent_impl_lambda.h"
#include "runtimeTypedExprComponent_impl_number.h"
#include "runtimeTypedExprComponent_impl_storage.h"
#include "runtimeTypeExprComponent_impl_utility.h"