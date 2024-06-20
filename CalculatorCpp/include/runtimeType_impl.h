#ifndef RUNTIMETYPE_IMPL
#define RUNTIMETYPE_IMPL

#include <iostream>
#include <vector>
#include <stack>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include <sstream>
#include <format>
#include <memory>
#include <sstream>
#include <cassert>

#include "runtime_error.h"
#include "runtimeType.h"
#include "result.h"
#include "lexer.h"

// Copy constructor for RuntimeCompoundType
inline RuntimeCompoundType::RuntimeCompoundType(const RuntimeCompoundType& other) :
	mHashed{ other.mHashed },
	Type{ other.Type },
	Children{ other.Children } {
	static size_t count{ 0 };
	//std::cout << "copy: " << ++count << " " << *this << "\n";
}

// Move constructor for RuntimeCompoundType
inline RuntimeCompoundType::RuntimeCompoundType(RuntimeCompoundType&& other) noexcept :
	mHashed{ other.mHashed },
	Type{ std::move(other.Type) },
	Children{ std::move(other.Children) } {}

// Copy assignment operator for RuntimeCompoundType
inline RuntimeCompoundType& RuntimeCompoundType::operator=(const RuntimeCompoundType& other) {
	if (this != &other) {
		Type = other.Type;
		Children = other.Children;
		mHashed = other.mHashed;
	}

	return *this;
}

// Move assignment operator for RuntimeCompoundType
inline RuntimeCompoundType& RuntimeCompoundType::operator=(RuntimeCompoundType&& other) noexcept {
	if (this != &other) {
		Type = std::move(other.Type);
		Children = std::move(other.Children);
		mHashed = other.mHashed;
	}
	return *this;
}

// Create a RuntimeCompoundType with a vector of RuntimeType elements
inline RuntimeCompoundType RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(const std::vector<RuntimeType>& base) {
	return RuntimeCompoundType(RuntimeBaseType::_Storage, base);
}

// Create a RuntimeCompoundType with a rvalue vector of RuntimeType elements
inline RuntimeCompoundType RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(std::vector<RuntimeType>&& base) {
	return RuntimeCompoundType(RuntimeBaseType::_Storage, std::move(base));
}

// Create a RuntimeCompoundType with variadic arguments of RuntimeType elements
template <RuntimeTypeRequired... Args>
inline RuntimeCompoundType RuntimeCompoundType::Storage(Args&&... base) {
	constexpr size_t count = sizeof...(base);
	std::vector<RuntimeType> tmp;
	tmp.reserve(count);
	(tmp.emplace_back(std::move(std::forward<Args>(base))), ...);
	return RuntimeCompoundType(RuntimeBaseType::_Storage, std::move(tmp));
}

// Create a RuntimeCompoundType with a vector of RuntimeType elements
inline RuntimeCompoundType RuntimeCompoundType::Storage(const std::vector<RuntimeType>& base) {
	for (const RuntimeType& element : base)
		assert(
			(std::holds_alternative<RuntimeBaseType>(element) && static_cast<int8_t>(std::get<RuntimeBaseType>(element)) > 3) || \
			(std::holds_alternative<RuntimeCompoundType>(element) && static_cast<int8_t>(std::get<RuntimeCompoundType>(element).Type) > 3)
		);

	return RuntimeCompoundType(RuntimeBaseType::_Storage, base);
}

// Create a RuntimeCompoundType with a rvalue vector of RuntimeType elements
inline RuntimeCompoundType RuntimeCompoundType::Storage(std::vector<RuntimeType>&& base) {
	for (const RuntimeType& element : base)
		assert(
			(std::holds_alternative<RuntimeBaseType>(element) && static_cast<int8_t>(std::get<RuntimeBaseType>(element)) > 3) || \
			(std::holds_alternative<RuntimeCompoundType>(element) && static_cast<int8_t>(std::get<RuntimeCompoundType>(element).Type) > 3)
		);

	return RuntimeCompoundType(RuntimeBaseType::_Storage, std::move(base));
}

// Create a RuntimeCompoundType representing a lambda with return type and parameters
inline RuntimeCompoundType RuntimeCompoundType::Lambda(const RuntimeType& Ret, const RuntimeType& Params) {
	return RuntimeCompoundType(RuntimeBaseType::_Lambda, { Ret, Params });
}

// Create a RuntimeCompoundType representing a lambda with return type and parameters
inline RuntimeCompoundType RuntimeCompoundType::Lambda(RuntimeType&& Ret, RuntimeType&& Params) {
	std::vector<RuntimeType> tmp;
	tmp.reserve(2);
	tmp.emplace_back(std::move(Ret));
	tmp.emplace_back(std::move(Params));
	return RuntimeCompoundType(RuntimeBaseType::_Lambda, std::move(tmp));
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaInfix(const RuntimeType& Ret) {
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Infix, { Ret });
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaInfix(RuntimeType&& Ret) {
	std::vector<RuntimeType> tmp;
	tmp.emplace_back(std::move(Ret));
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Infix, std::move(tmp));
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaPostfix(const RuntimeType& Ret) {
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Postfix, { Ret });
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaPostfix(RuntimeType&& Ret) {
	std::vector<RuntimeType> tmp;
	tmp.emplace_back(std::move(Ret));
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Postfix, std::move(tmp));
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaPrefix(const RuntimeType& Ret) {
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Prefix, { Ret });
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaPrefix(RuntimeType&& Ret) {
	std::vector<RuntimeType> tmp;
	tmp.emplace_back(std::move(Ret));
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Prefix, std::move(tmp));
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaConstant(const RuntimeType& Ret) {
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Prefix, { Ret });
}

inline RuntimeCompoundType RuntimeCompoundType::RuntimeEvaluateLambdaConstant(RuntimeType&& Ret) {
	std::vector<RuntimeType> tmp;
	tmp.emplace_back(std::move(Ret));
	return RuntimeCompoundType(RuntimeBaseType::_Operator_Lambda_Constant, std::move(tmp));
}

// Parse a string representing a RuntimeType
inline Result<RuntimeType, std::runtime_error> RuntimeCompoundType::ParseString(const std::string& stringLikeType) {
	// Initialize the lexer if not already done
	static Lexer lex;
	static bool initialized = false;

	if (!initialized)
		lex.setKeywords({ "Number", "NodePointer",  "Storage", "Lambda", "[", "]" });

	// Tokenize the input string using the lexer
	std::vector<std::string> lexemes = lex.lexing(stringLikeType).getValue();
	std::stack<std::variant<char, RuntimeType>> operationStack;

	// Process each lexeme
	for (const std::string& lexeme : lexemes) {
		// Handle closing bracket
		if (lexeme == "]") {
			// Collect elements until an opening bracket is found
			std::vector<RuntimeType> containRuntimeType;
			while (!operationStack.empty() && !std::holds_alternative<char>(operationStack.top())) {
				containRuntimeType.push_back(std::get<RuntimeType>(operationStack.top())); operationStack.pop();
			}
			if (operationStack.size() < 2)
				return std::runtime_error("");

			// Pop the opening bracket
			operationStack.pop();

			// Check the type of the element before the opening bracket
			if (std::holds_alternative<char>(operationStack.top()) ||
				std::holds_alternative<RuntimeCompoundType>(std::get<RuntimeType>(operationStack.top())) ||
				std::get<RuntimeBaseType>(std::get<RuntimeType>(operationStack.top())) == RuntimeBaseType::Number)
				return RuntimeError<RuntimeTypeError>("Holder of RuntimeTypes must be RuntimeBaseType::_Lambda or RuntimeBaseType::_Storage.", "RuntimeCompoundType::ParseString");

			// Determine the type of the element and create the corresponding RuntimeCompoundType
			RuntimeBaseType runtimeTypeElementHolder = std::get<RuntimeBaseType>(std::get<RuntimeType>(operationStack.top()));
			operationStack.pop();

			if (runtimeTypeElementHolder == RuntimeBaseType::_Storage)
				operationStack.emplace(RuntimeCompoundType::gurantreeNoRuntimeEvaluateStorage(containRuntimeType));

			else if (runtimeTypeElementHolder == RuntimeBaseType::_Lambda && containRuntimeType.size() == 2)
				operationStack.emplace(RuntimeCompoundType::Lambda(containRuntimeType[0], containRuntimeType[1]));
			else return RuntimeError<RuntimeTypeError>("RuntimeBaseType::_Lambda can hold only 2 argument (including Return and Params), use Storage if you want to return compoundType.", "RuntimeCompoundType::ParseString");
		}

		// Handle opening bracket
		else if (lexeme == "[")
			operationStack.emplace('[');

		// Handle other types
		else {
			if (lexeme == "Number")
				operationStack.emplace(RuntimeBaseType::Number);
			else if (lexeme == "NodePointer")
				operationStack.emplace(RuntimeBaseType::NodePointer);
			else if (lexeme == "Storage")
				operationStack.emplace(RuntimeBaseType::_Storage);
			else
				operationStack.emplace(RuntimeBaseType::_Lambda);
		}
	}

	// Check the final state of the stack
	if (operationStack.size() != 1 || std::holds_alternative<char>(operationStack.top()))
		return RuntimeError<RuntimeTypeError>("Failed to evaluate RuntimeType like string.", "RuntimeCompoundType::ParseString");

	return std::get<RuntimeType>(operationStack.top());
}

// Get the storage information from a RuntimeType representing storage
inline RuntimeCompoundType::StorageInfo RuntimeCompoundType::getStorageInfo(const RuntimeType& storage) {
	assert(std::get_if<RuntimeCompoundType>(&storage)->Type == RuntimeBaseType::_Storage);

	const RuntimeCompoundType& storageCompound = std::get<RuntimeCompoundType>(storage);

	return StorageInfo{
		std::make_shared<std::vector<RuntimeType>>(storageCompound.Children),
		storageCompound.Children.size()
	};
}

// Get the lambda information from a RuntimeType representing lambda
inline RuntimeCompoundType::LambdaInfo RuntimeCompoundType::getLambdaInfo(const RuntimeType& lambda) {
	assert(std::get_if<RuntimeCompoundType>(&lambda)->Type == RuntimeBaseType::_Lambda);

	const RuntimeCompoundType& lambdaCompound = std::get<RuntimeCompoundType>(lambda);

	if (std::holds_alternative<RuntimeCompoundType>(lambdaCompound.Children[1]))
		return LambdaInfo{
			std::make_shared<RuntimeType>(lambdaCompound.Children[0]),
			std::make_shared<RuntimeType>(lambdaCompound.Children[1]),
			getStorageInfo(lambdaCompound.Children[1]).StorageSize
	};

	if (std::get<RuntimeBaseType>(lambdaCompound.Children[1]) == RuntimeBaseType::_Storage)
		return LambdaInfo{
			std::make_shared<RuntimeType>(lambdaCompound.Children[0]),
			std::make_shared<RuntimeType>(lambdaCompound.Children[1]),
			0
	};

	return LambdaInfo{
		std::make_shared<RuntimeType>(lambdaCompound.Children[0]),
		std::make_shared<RuntimeType>(lambdaCompound.Children[1]),
		1
	};
}

inline RuntimeType RuntimeCompoundType::_getLambdaParamsType(const RuntimeType& lambda) {
	assert(std::get_if<RuntimeCompoundType>(&lambda)->Type == RuntimeBaseType::_Lambda);

	const RuntimeCompoundType& lambdaCompound = std::get<RuntimeCompoundType>(lambda);
	return lambdaCompound.Children[1];
}

inline RuntimeType RuntimeCompoundType::_getLambdaReturnType(const RuntimeType& lambda) {
	assert(std::get_if<RuntimeCompoundType>(&lambda)->Type == RuntimeBaseType::_Lambda);

	const RuntimeCompoundType& lambdaCompound = std::get<RuntimeCompoundType>(lambda);
	return lambdaCompound.Children[0];
}

inline size_t RuntimeCompoundType::_getLambdaParamsNumbers(const RuntimeType& lambda) {
	assert(std::get_if<RuntimeCompoundType>(&lambda)->Type == RuntimeBaseType::_Lambda);

	const RuntimeCompoundType& lambdaCompound = std::get<RuntimeCompoundType>(lambda);

	if (std::holds_alternative<RuntimeCompoundType>(lambdaCompound.Children[1])) {
		assert(std::get_if<RuntimeCompoundType>(&lambdaCompound.Children[1])->Type == RuntimeBaseType::_Storage);
		return std::get<RuntimeCompoundType>(lambdaCompound.Children[1]).Children.size();
	}

	if (std::get<RuntimeBaseType>(lambdaCompound.Children[1]) == RuntimeBaseType::_Storage)
		return 0;

	return 1;
}

inline size_t RuntimeCompoundType::_getHash() const {
	return mHashed;
}

// Equality operator for RuntimeType, use hash to compare types.
inline bool operator==(const RuntimeType& runtimeType1, const RuntimeType& runtimeType2) {
	return std::hash<RuntimeType>{}(runtimeType1) == std::hash<RuntimeType>{}(runtimeType2);
}

// Output operator for RuntimeBaseType
inline std::ostream& operator<<(std::ostream& os, const RuntimeBaseType& ebt) {
	switch (ebt) {
	case RuntimeBaseType::Number:
		os << "Number"; break;
	case RuntimeBaseType::NodePointer:
		os << "NodePointer"; break;
	case RuntimeBaseType::_Lambda:
		os << "NULL_Lambda"; break;
	case RuntimeBaseType::_Storage:
		os << "NULL_Storage"; break;
	}
	return os;
}

// Output operator for RuntimeType
inline std::ostream& operator<<(std::ostream& os, const RuntimeType& et) {
	if (const RuntimeBaseType* runtimeBaseType{ std::get_if<RuntimeBaseType>(&et) }; runtimeBaseType)
		os << *runtimeBaseType;

	else if (const RuntimeCompoundType* runtimeCompoundType{ std::get_if<RuntimeCompoundType>(&et) }; runtimeCompoundType) {
		switch (runtimeCompoundType->Type) {
		case RuntimeBaseType::Number:
			os << "ERROR_Number"; break;
		case RuntimeBaseType::NodePointer:
			os << "ERROR_NodePointer"; break;
		case RuntimeBaseType::_Lambda:
			os << "Lambda"; break;
		case RuntimeBaseType::_Storage:
			os << "Storage"; break;
		}

		os << "(";
		for (const auto& child : runtimeCompoundType->Children)
			os << child << ", ";
		os << "\b\b)";
	}
	else {
		os << "wtf";
	}

	return os;
}

inline std::string RuntimeTypeToString(const RuntimeType& rt) {
	std::ostringstream oss;
	if (const RuntimeBaseType* runtimeBaseType{ std::get_if<RuntimeBaseType>(&rt) }; runtimeBaseType)
		oss << *runtimeBaseType;

	else if (const RuntimeCompoundType* runtimeCompoundType{ std::get_if<RuntimeCompoundType>(&rt) }; runtimeCompoundType) {
		switch (runtimeCompoundType->Type) {
		case RuntimeBaseType::Number:
			oss << "ERROR_Number"; break;
		case RuntimeBaseType::NodePointer:
			oss << "ERROR_NodePointer"; break;
		case RuntimeBaseType::_Lambda:
			oss << "Lambda"; break;
		case RuntimeBaseType::_Storage:
			oss << "Storage"; break;
		}

		oss << "(";
		for (const auto& child : runtimeCompoundType->Children)
			oss << child << ", ";
		oss << "\b\b)";
	}
	else {
		oss << "wtf";
	}

	return oss.str();
}

// Output operator for RuntimeCompoundType
inline std::ostream& operator<<(std::ostream& os, const RuntimeCompoundType& ect) {
	switch (ect.Type) {
	case RuntimeBaseType::Number:
		os << "ERROR_Number"; break;
	case RuntimeBaseType::NodePointer:
		os << "ERROR_NodePointer"; break;
	case RuntimeBaseType::_Lambda:
		os << "Lambda"; break;
	case RuntimeBaseType::_Storage:
		os << "Storage"; break;
	}

	os << "(";
	for (const auto& child : ect.Children)
		os << child << ", ";
	os << "\b\b)";

	return os;
}

inline size_t RuntimeCompoundType::generateHash(RuntimeBaseType wrapper, const std::vector<RuntimeType>& base) const {
	size_t seed = 0;

	// Hash the base type
	hash_combine(seed, wrapper);

	// Hash the children
	for (const auto& child : base) {
		if (std::holds_alternative<RuntimeCompoundType>(child))
			hash_combine(seed, std::get<RuntimeCompoundType>(child).mHashed);
		else if (std::holds_alternative<RuntimeBaseType>(child))
			hash_combine(seed, std::hash<char>{}(static_cast<char>(std::get<RuntimeBaseType>(child))));
	}
	return seed;
}

inline size_t RuntimeCompoundType::generateHash(RuntimeBaseType wrapper, const RuntimeType& base) const {
	size_t seed = 0;

	// Hash the base type
	hash_combine(seed, wrapper);

	// Hash the children
	if (std::holds_alternative<RuntimeCompoundType>(base))
		hash_combine(seed, std::get<RuntimeCompoundType>(base).mHashed);
	else if (std::holds_alternative<RuntimeBaseType>(base))
		hash_combine(seed, std::hash<char>{}(static_cast<char>(std::get<RuntimeBaseType>(base))));

	return seed;
}

template <typename T>
inline void hash_combine(size_t& seed, const T& v) {
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline std::size_t std::hash<RuntimeType>::operator()(const RuntimeType& rt) const {
	if (std::holds_alternative<RuntimeBaseType>(rt))
		return std::hash<char>{}(static_cast<char>(std::get<RuntimeBaseType>(rt)));
	return std::get<RuntimeCompoundType>(rt)._getHash();
}

// Formatter for RuntimeType
template <>
struct std::formatter<RuntimeType> : std::formatter<std::string> {
	auto format(RuntimeType rtt, format_context& ctx) const {
		std::ostringstream ss;
		if (std::holds_alternative<RuntimeCompoundType>(rtt))
			ss << std::get<RuntimeCompoundType>(rtt);
		else
			ss << std::get<RuntimeBaseType>(rtt);

		return formatter<string>::format(
			std::format("{}", ss.str()), ctx);
	}
};

#endif // RUNTIMETYPE_IMPL
