// Ensures inclusion only once
#pragma once

#include <iostream>
#include <vector>    
#include <stack>     
#include <variant>  
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <format>
#include "colorText.h"
#include "result.h"

struct RuntimeTypeError {
    static const std::string prefix;
};
inline const std::string RuntimeTypeError::prefix = "RuntimeTypeError";

[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// Enum representing the base types that can be stored in RuntimeTypedExprComponent
enum class RuntimeBaseType : int8_t {
    Number,  // Numeric type
    NodePointer, // Unsigned interget pointed to specific node
    _Lambda, // Internal use for representing lambda functions
    _Storage, // Internal use for representing storage types (collections)
    _Operator_Lambda_Infix,
    _Operator_Lambda_Postfix,
    _Operator_Lambda_Prefix,
    _Operator_Lambda_Constant,
    _Stroage_Any,
};

// Forward declaration for RuntimeCompoundType (used later in RuntimeType)
class RuntimeCompoundType;

// Type alias for RuntimeType using std::variant for multiple data types
using RuntimeType = std::variant<RuntimeBaseType, RuntimeCompoundType>;

// Concept to ensure types used with RuntimeType are compatible
template <typename T>
concept RuntimeTypeRequired = std::same_as<T, RuntimeCompoundType> || std::same_as<T, RuntimeBaseType>;

// Class representing a compound type (storage or lambda)
class RuntimeCompoundType {
private:
    // Information about a lambda function
    struct LambdaInfo {
        std::shared_ptr<RuntimeType> ReturnType;    // Pointer to the return type
        std::shared_ptr<RuntimeType> ParamsType;    // Pointer to the parameter type
        size_t ParamsNumbers;                       // Number of parameters
    };

    // Information about a storage type (collection)
    struct StorageInfo {
        std::shared_ptr<std::vector<RuntimeType>> Storage;  // Pointer to the vector of elements
        size_t StorageSize;                                 // Size of the storage
    };

    // Hash for comparing RuntimeTypes
    size_t mHashed;

public:
    // Base type of the compound type (storage or lambda)
    RuntimeBaseType Type;
    // Child types within the compound type (e.g., element types in storage)
    std::vector<RuntimeType> Children;

    // Copy constructor
    RuntimeCompoundType(const RuntimeCompoundType& other);

    // Move constructor (avoids unnecessary copying)
    RuntimeCompoundType(RuntimeCompoundType&& other) noexcept;

    // Copy assignment operator
    RuntimeCompoundType& operator=(const RuntimeCompoundType& other);

    // Move assignment operator
    RuntimeCompoundType& operator=(RuntimeCompoundType&& other) noexcept;

    // Static factory methods to create different compound types
    // * Storage - Creates a storage type from arguments (variadic template)
    // * Storage - Creates a storage type from a vector of RuntimeTypes
    // * Lambda - Creates a lambda type with return and parameter types
    // * ParseString - Parses a string representation into a RuntimeType

    static RuntimeCompoundType gurantreeNoRuntimeEvaluateStorage(const std::vector<RuntimeType>& base);
    static RuntimeCompoundType gurantreeNoRuntimeEvaluateStorage(std::vector<RuntimeType>&& base);
    template <RuntimeTypeRequired... Args>
    static RuntimeCompoundType Storage(Args&&... base);
    static RuntimeCompoundType Storage(const std::vector<RuntimeType>& base);
    static RuntimeCompoundType Storage(std::vector<RuntimeType>&& base);
    static RuntimeCompoundType Lambda(const RuntimeType& Ret, const RuntimeType& Params);
    static RuntimeCompoundType Lambda(RuntimeType&& Ret, RuntimeType&& Params);

    static RuntimeCompoundType RuntimeEvaluateLambdaInfix(const RuntimeType& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaInfix(RuntimeType&& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaPostfix(const RuntimeType& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaPostfix(RuntimeType&& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaPrefix(const RuntimeType& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaPrefix(RuntimeType&& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaConstant(const RuntimeType& Ret);
    static RuntimeCompoundType RuntimeEvaluateLambdaConstant(RuntimeType&& Ret);

    static Result<RuntimeType, std::runtime_error> ParseString(const std::string& stringLikeType);

    // Funtion allow acessing mHashed
    size_t _getHash() const;

    // Function to extract information about a storage type from a RuntimeType object
    static StorageInfo getStorageInfo(const RuntimeType& storage);

    // Function to extract information about a lambda function from a RuntimeType object
    static LambdaInfo getLambdaInfo(const RuntimeType& lambda);

    // Friend Function to compare two RuntimeType objects for equality
    friend bool operator==(const RuntimeType& runtimeType1, const RuntimeType& runtimeType2);

    // Friend function to allow streaming RuntimeType objects to ostream
    friend std::ostream& operator<<(std::ostream& os, const RuntimeType& et);

    // Friend function to allow streaming RuntimeCompoundType objects to ostream
    friend std::ostream& operator<<(std::ostream& os, const RuntimeCompoundType& ect);

    // Non Function to extract spectify information from lambda.
    static RuntimeType _getLambdaParamsType(const RuntimeType& lambda);
    static RuntimeType _getLambdaReturnType(const RuntimeType& lambda);
    static size_t _getLambdaParamsNumbers(const RuntimeType& lambda);

    // Friend declarations for classes that might interact with RuntimeCompoundType or its nested types
    friend class Lambda;
    friend class Storage;

private:
    size_t generateHash(RuntimeBaseType wrapper, const std::vector<RuntimeType>& base) const;
    size_t generateHash(RuntimeBaseType wrapper, const RuntimeType& base) const;

    // Private constructor for compound types (used by factory methods)
    RuntimeCompoundType(RuntimeBaseType wrapper, const std::vector<RuntimeType>& base) :
        mHashed{ generateHash(wrapper, base) },
        Type{ wrapper },
        Children{ base } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, std::vector<RuntimeType>&& base) :
        mHashed{ generateHash(wrapper, base) },
        Type{ wrapper },
        Children{ std::move(base) } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, const RuntimeType& base) :
        mHashed{ generateHash(wrapper, {base}) },
        Type{ wrapper },
        Children{ {base} } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, RuntimeType&& base) :
        mHashed{ generateHash(wrapper, base) },
        Type{ wrapper },
        Children{ {std::move(base)} } {}
};

std::string RuntimeTypeToString(const RuntimeType& rt);

template <typename T>
void hash_combine(size_t& seed, const T& v);

template <>
struct std::hash<RuntimeType> {
    std::size_t operator()(const RuntimeType& rt) const;
};

#include "runtimeType_impl.h"

