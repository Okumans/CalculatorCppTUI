#ifndef TYPE_DEMO
#define TYPE_DEMO

#include "result.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <span>
#include <type_traits>
#include <vector>

/////////////////////////////// Utility functions //////////////////////////////

[[noreturn]] inline void unreachable() {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

template <typename T> class SharedSpan {
public:
    SharedSpan(std::shared_ptr<T[]> storage, std::size_t size)
        : mStorage(std::move(storage)), mSpan(mStorage.get(), size) {}

    SharedSpan(T* storage, std::size_t size)
        : mStorage(storage, [](T*) {}), mSpan(mStorage.get(), size) {}

    SharedSpan(std::shared_ptr<T[]> storage, std::size_t start, std::size_t size)
        : mStorage(storage), mSpan(mStorage.get() + start, size) {}

    std::span<T> getSpan() const { return mSpan; }
    constexpr std::span<T> cgetSpan() const { return mSpan; }

    std::shared_ptr<T[]> getShared() const { return mStorage; }

private:
    std::shared_ptr<T[]> mStorage;
    std::span<T> mSpan;
};

////////////////////////////////////////////////////////////////////////////////

enum class RuntimeBaseType : uint8_t;
class RuntimeType;

template <typename T>
concept RuntimeTypedRequired =
std::is_same_v<std::remove_cvref_t<T>, RuntimeBaseType> ||
std::is_same_v<std::remove_cvref_t<T>, RuntimeType>;

#pragma pack(push, 1) // Ensure no padding is added
struct BitFieldRuntimeCompoundType {
private:
    uint8_t mStroableType : 1;
    uint8_t mBaseRuntimeType : 4;
    uint32_t mJumpToEnd : 27;

public:
    constexpr BitFieldRuntimeCompoundType();
    constexpr BitFieldRuntimeCompoundType(bool isStorableType,
        RuntimeBaseType type,
        uint32_t jumpToEnd);
    constexpr bool isStorableType() const;
    constexpr RuntimeBaseType getBaseRuntimeType() const;
    constexpr uint32_t getJumpEnd() const;

    constexpr bool operator==(const BitFieldRuntimeCompoundType& other) const;
    constexpr bool operator!=(const BitFieldRuntimeCompoundType& other) const;
};
#pragma pack(pop) // Reset the packing alignment

// Enum representing the base types that can be stored in
// RuntimeTypedExprComponent
enum class RuntimeBaseType : uint8_t {
    Number,      // Numeric type
    NodePointer, // Unsigned interget pointed to specific node
    _Lambda,     // Internal use for representing lambda functions
    _Storage,    // Internal use for representing storage types (collections)
    _Operator_Lambda_Infix,
    _Operator_Lambda_Postfix,
    _Operator_Lambda_Prefix,
    _Operator_Lambda_Constant,
    _Stroage_Any,
};

const char* runtimeBaseTypeToString(RuntimeBaseType type);

class RuntimeTypeLookUp {
public:
    class Iterator {
    public:
        // Type definitions required for an iterator
        using iterator_category = std::forward_iterator_tag;
        using value_type = BitFieldRuntimeCompoundType;
        using difference_type = std::ptrdiff_t;
        using pointer = const BitFieldRuntimeCompoundType*;
        using reference = const BitFieldRuntimeCompoundType&;

        constexpr Iterator(pointer ptr);
        constexpr reference operator*() const;
        constexpr pointer operator->() const;
        constexpr Iterator& operator++();
        constexpr Iterator operator++(int);
        constexpr bool operator==(const Iterator& other) const;
        constexpr bool operator!=(const Iterator& other) const;
        constexpr RuntimeTypeLookUp::Iterator::pointer get() const;
        RuntimeTypeLookUp lookUp() const;
        RuntimeTypeLookUp lookUpChildren() const;
        virtual RuntimeType toRuntimeType() const;

    protected:
        pointer mPtr;
    };

    explicit constexpr RuntimeTypeLookUp(const RuntimeType& runtimeType);
    explicit constexpr RuntimeTypeLookUp(
        const std::span<const BitFieldRuntimeCompoundType>& runtimeTypeRawData);
    explicit constexpr RuntimeTypeLookUp(Iterator::pointer begin, size_t count);
    constexpr Iterator begin() const;
    constexpr Iterator end() const;
    virtual std::vector<RuntimeType> toRuntimeTypes() const;

protected:
    const std::span<const BitFieldRuntimeCompoundType> mData;
};

class HeavyRuntimeTypeLookUp : public RuntimeTypeLookUp {
public:
    class Iterator : public RuntimeTypeLookUp::Iterator {
    public:
        constexpr Iterator(const HeavyRuntimeTypeLookUp& outterClass, pointer ptr);
        HeavyRuntimeTypeLookUp heavyLookUp() const;
        HeavyRuntimeTypeLookUp heavyLookUpChildren() const;
        RuntimeType toRuntimeType() const override;

    private:
        const HeavyRuntimeTypeLookUp& outer;
    };

    explicit inline HeavyRuntimeTypeLookUp(const RuntimeType& runtimeType);
    explicit HeavyRuntimeTypeLookUp(
        std::shared_ptr<BitFieldRuntimeCompoundType[]> sharedPtr,
        Iterator::pointer begin, size_t count);
    std::vector<RuntimeType> toRuntimeTypes() const override;

    constexpr Iterator begin() const;
    constexpr Iterator end() const;

private:
    std::shared_ptr<BitFieldRuntimeCompoundType[]> mSharedPtr;
};

class LambdaTypeLookUp : private HeavyRuntimeTypeLookUp {
public:
    LambdaTypeLookUp(const RuntimeType& runtimeType);
    LambdaTypeLookUp(const HeavyRuntimeTypeLookUp& heavyLookUp);
    RuntimeType returnType() const;
    RuntimeType paramsType() const;
    size_t paramsSize() const;
    RuntimeTypeLookUp lookUpReturnType() const;
    RuntimeTypeLookUp lookUpParamsType() const;
    HeavyRuntimeTypeLookUp heavyLookUpReturnType() const;
    HeavyRuntimeTypeLookUp heavyLookUpParamsType() const;
};

class StorageTypeLookUp : private HeavyRuntimeTypeLookUp {
public:
    StorageTypeLookUp(const RuntimeType& runtimeType);
    StorageTypeLookUp(const HeavyRuntimeTypeLookUp& heavyLookUp);
    RuntimeType at(size_t index) const;
    RuntimeTypeLookUp lookUpAt(size_t index) const;
    HeavyRuntimeTypeLookUp heavyLookUpAt(size_t index) const;
    size_t size() const;

private:
    size_t mSize;
    size_t get_initailize_size(HeavyRuntimeTypeLookUp& outer,
        Iterator firstChildrenIterator) const;
};

class RuntimeType {
public:
    // defualt constructor
    RuntimeType();
    RuntimeType(const SharedSpan<BitFieldRuntimeCompoundType>& type);
    RuntimeType(SharedSpan<BitFieldRuntimeCompoundType>&& type);
    RuntimeType(const BitFieldRuntimeCompoundType& type);
    RuntimeType(RuntimeBaseType type);

    // Create a Storage RuntimeType with the guarantee that none of its inner
    // types will be RuntimeEvaluate.
    static RuntimeType
        gurantreeNoRuntimeEvaluateStorage(const std::vector<RuntimeType>& base);

    // Create a Storage RuntimeType
    template <RuntimeTypedRequired... Args>
    static RuntimeType Storage(Args &&...base);
    static RuntimeType Storage(const std::vector<RuntimeType>& base);

    // Create a Lambda RuntimeType
    static RuntimeType Lambda(const RuntimeType& Ret, const RuntimeType& Params);
    static RuntimeType Lambda(RuntimeType&& Ret, RuntimeType&& Params);

    // BaseRuntimeType
    static const RuntimeType Number;
    static const RuntimeType NodePointer;

    class HiddenType {
        // private baseRuntimeType
    public:
        static const RuntimeType _Stroage_Any;
        static const RuntimeType _Storage;
        static const RuntimeType _Lambda;

        // Create a Operator Lambda Infix RuntimeType
        static RuntimeType _Operator_Lambda_Infix(const RuntimeType& Ret);
        static RuntimeType _Operator_Lambda_Infix(RuntimeType&& Ret);

        // Create a Operator Lambda Postfix RuntimeType
        static RuntimeType _Operator_Lambda_Postfix(const RuntimeType& Ret);
        static RuntimeType _Operator_Lambda_Postfix(RuntimeType&& Ret);

        // Create a Operator Lambda Prefix RuntimeType
        static RuntimeType _Operator_Lambda_Prefix(const RuntimeType& Ret);
        static RuntimeType _Operator_Lambda_Prefix(RuntimeType&& Ret);

        // Create a Operator Lambda Constant RuntimeType
        static RuntimeType _Operator_Lambda_Constant(const RuntimeType& Ret);
        static RuntimeType _Operator_Lambda_Constant(RuntimeType&& Ret);
    };

    static Result<RuntimeType, std::runtime_error>
        ParseString(const std::string& stringLikeType);

    const std::span<const BitFieldRuntimeCompoundType> getRawType() const;
    RuntimeBaseType getBaseType() const;
    std::vector<RuntimeType> getChildren() const;
    RuntimeTypeLookUp lookUpChildren() const;
    HeavyRuntimeTypeLookUp heavyLookUpChildren() const;
    LambdaTypeLookUp asLambda() const;
    StorageTypeLookUp asStorage() const;
    std::string toString() const;
    bool operator==(const RuntimeType& other) const;
    bool operator!=(const RuntimeType& other) const;

    friend class RuntimeTypeLookUp;
    friend class HeavyRuntimeTypeLookUp;
    friend class Lambda;
    friend std::ostream& operator<<(std::ostream& os,
        const RuntimeType& runtimeType);

private:
    // member variables
    SharedSpan<BitFieldRuntimeCompoundType> mType;

private:
    // private baseRuntimeType
    //static const RuntimeType _Stroage_Any;
    //static const RuntimeType _Storage;
    //static const RuntimeType _Lambda;

    SharedSpan<BitFieldRuntimeCompoundType> static convertToRuntimeType(
        RuntimeBaseType base,
        std::vector<RuntimeType>::const_iterator children_begin,
        std::vector<RuntimeType>::const_iterator children_end);

    SharedSpan<BitFieldRuntimeCompoundType> static convertToRuntimeType(
        const RuntimeType &base, const std::vector<RuntimeType>& children);

    SharedSpan<BitFieldRuntimeCompoundType> static convertToRuntimeType(
        RuntimeBaseType base, const std::vector<RuntimeType>& children);

    SharedSpan<BitFieldRuntimeCompoundType> static convertToRuntimeType(
        const std::vector<RuntimeType>& fullType);
};

#include "runtimeType_demo_impl.h"
#endif // TYPE_DEMO
