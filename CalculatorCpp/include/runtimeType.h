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

#include "result.h"

// Custom exception class for runtime type errors
class RuntimeTypeError : public std::runtime_error {
public:
    // Constructor with a single message
    explicit RuntimeTypeError(const std::string& message)
        : std::runtime_error("RuntimeTypeError: " + message) {}

    // Constructor with message and origin information
    explicit RuntimeTypeError(const std::string& message, const std::string from)
        : std::runtime_error("RuntimeTypeError: " + message + " (from: " + from + ")") {}

    // Constructor with chained error, message, and origin information
    explicit RuntimeTypeError(const std::runtime_error& baseError, const std::string& message, const std::string from)
        : std::runtime_error("RuntimeTypeError: " + message + " (from: " + from + ") chained from " + baseError.what()) {}
};

// Enum representing the base types that can be stored in RuntimeTypedExprComponent
enum class RuntimeBaseType : char {
    Number,  // Numeric type
    _Lambda, // Internal use for representing lambda functions
    _Storage, // Internal use for representing storage types (collections)
};

// Forward declaration for RuntimeCompoundType (used later in RuntimeType)
class RuntimeCompoundType;

// Type alias for RuntimeType using std::variant for multiple data types
using RuntimeType = std::variant<RuntimeBaseType, RuntimeCompoundType>;

// Concept to ensure types used with RuntimeType are compatible
template <typename T>
concept RuntimeTypeRequired = std::same_as<T, RuntimeCompoundType> || std::same_as<T, RuntimeBaseType> || std::same_as<T, RuntimeType>;

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

    template <RuntimeTypeRequired... Args>
    static RuntimeCompoundType Storage(Args&&... base);
    static RuntimeCompoundType Storage(const std::vector<RuntimeType>& base);
    static RuntimeCompoundType Storage(std::vector<RuntimeType>&& base);
    static RuntimeCompoundType Lambda(const RuntimeType& Ret, const RuntimeType& Params);
    static RuntimeCompoundType Lambda(RuntimeType&& Ret, RuntimeType&& Params);
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

    // Friend declarations for classes that might interact with RuntimeCompoundType or its nested types
    friend class Lambda;
    friend class Storage;

private:
    size_t generateHash(RuntimeBaseType wrapper, const std::vector<RuntimeType>& base) const;
    size_t generateHash(RuntimeBaseType wrapper, const RuntimeType& base) const;

    // Non Function to extract spectify information from lambda.
    static RuntimeType _getLambdaParamsType(const RuntimeType& lambda);
    static RuntimeType _getLambdaReturnType(const RuntimeType& lambda);
    static size_t _getLambdaParamsNumbers(const RuntimeType& lambda);

    // Private constructor for compound types (used by factory methods)
    RuntimeCompoundType(RuntimeBaseType wrapper, const std::vector<RuntimeType>& base) :
        Type{ wrapper },
        Children{ base },
        mHashed{ generateHash(wrapper, base) } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, std::vector<RuntimeType>&& base) :
        Type{ wrapper },
        mHashed{ generateHash(wrapper, base) },
        Children{ std::move(base) } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, const RuntimeType& base) :
        Type{ wrapper },
        Children{ {base} },
        mHashed{ generateHash(wrapper, {base}) } {}

    RuntimeCompoundType(RuntimeBaseType wrapper, RuntimeType&& base) :
        Type{ wrapper },
        mHashed{ generateHash(wrapper, base) },
        Children{ {std::move(base)} } {}
};

template <typename T>
void hash_combine(size_t& seed, const T& v);

template <>
struct std::hash<RuntimeType> {
    std::size_t operator()(const RuntimeType& rt) const;
};

#include "runtimeType_impl.h"
