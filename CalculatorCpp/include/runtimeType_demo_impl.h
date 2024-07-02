#ifndef TYPE_DEMO_IMPL
#define TYPE_DEMO_IMPL

#include "lexer.h"
#include "runtime_error.h"
#include "runtimeType_demo.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>
#include <format>

// implements fake definiton for static RuntimeTYpe
inline const RuntimeType RuntimeType::Number{ RuntimeType(RuntimeBaseType::Number) };

inline const RuntimeType RuntimeType::NodePointer{
    RuntimeType(RuntimeBaseType::NodePointer) };

inline const RuntimeType RuntimeType::HiddenType::_Stroage_Any{
    RuntimeType(RuntimeBaseType::_Stroage_Any) };

inline const RuntimeType RuntimeType::HiddenType::_Storage{ RuntimeType(RuntimeBaseType::_Storage) };

inline const RuntimeType RuntimeType::HiddenType::_Lambda{ RuntimeType(RuntimeBaseType::_Lambda) };

/////////////////////////////// RuntimeBaseType ////////////////////////////////

inline const char* runtimeBaseTypeToString(RuntimeBaseType type) {
    switch (type) {
        using enum RuntimeBaseType;
    case Number:
        return "Number";
    case NodePointer:
        return "NodePointer";
    case _Lambda:
        return "Lambda";
    case _Storage:
        return "Storage";
    case _Operator_Lambda_Infix:
        return "_Operator_Lambda_Infix";
    case _Operator_Lambda_Postfix:
        return "_Operator_Lambda_Postfix";
    case _Operator_Lambda_Prefix:
        return "_Operator_Lambda_Prefix";
    case _Operator_Lambda_Constant:
        return "_Operator_Lambda_Constant";
    case _Stroage_Any:
        return "_Stroage_Any";
    }
    unreachable();
}

/////////////////////////////// BitFieldRuntimeCompoundType ////////////////////

// implements BitFieldRuntimeCompoundType constructor
constexpr BitFieldRuntimeCompoundType::BitFieldRuntimeCompoundType(
    bool isStorableType, RuntimeBaseType type, uint32_t jumpToEnd)
    : mStroableType{ isStorableType },
    mBaseRuntimeType{ static_cast<uint8_t>(type) }, mJumpToEnd{ jumpToEnd } {}

constexpr BitFieldRuntimeCompoundType::BitFieldRuntimeCompoundType()
    : mStroableType{ false },
    mBaseRuntimeType{ static_cast<uint8_t>(RuntimeBaseType::Number) },
    mJumpToEnd{ 0 } {}

constexpr bool BitFieldRuntimeCompoundType::isStorableType() const {
    return mStroableType;
}

constexpr RuntimeBaseType
BitFieldRuntimeCompoundType::getBaseRuntimeType() const {
    return static_cast<RuntimeBaseType>(mBaseRuntimeType);
}

constexpr uint32_t BitFieldRuntimeCompoundType::getJumpEnd() const {
    return mJumpToEnd;
}

constexpr bool BitFieldRuntimeCompoundType::operator==(
    const BitFieldRuntimeCompoundType& other) const {
    return (mStroableType == other.mStroableType &&
        mBaseRuntimeType == other.mBaseRuntimeType &&
        mJumpToEnd == other.mJumpToEnd);
}

constexpr bool BitFieldRuntimeCompoundType::operator!=(
    const BitFieldRuntimeCompoundType& other) const {
    return (mStroableType != other.mStroableType ||
        mBaseRuntimeType != other.mBaseRuntimeType ||
        mJumpToEnd != other.mJumpToEnd);
}

/////////////////////////////// RuntimeTypeLookUp //////////////////////////////

constexpr RuntimeTypeLookUp::RuntimeTypeLookUp(const RuntimeType& runtimeType)
    : mData{ runtimeType.mType.cgetSpan() } {}

constexpr RuntimeTypeLookUp::RuntimeTypeLookUp(
    const std::span<const BitFieldRuntimeCompoundType>& runtimeTypeRawData)
    : mData{ runtimeTypeRawData } {}

constexpr RuntimeTypeLookUp::RuntimeTypeLookUp(Iterator::pointer begin,
    size_t count)
    : mData(begin, count) {}

constexpr RuntimeTypeLookUp::Iterator RuntimeTypeLookUp::begin() const {
    return Iterator(std::to_address(mData.begin()));
}

constexpr RuntimeTypeLookUp::Iterator RuntimeTypeLookUp::end() const {
    return Iterator(std::to_address(mData.end()));
}

// This is expensive because we lack access to shared_ptr.
// Consider using the getChildren method in RuntimeType or use toRuntimeTypes
// via HeavyRUntimeType instead.
inline std::vector<RuntimeType> RuntimeTypeLookUp::toRuntimeTypes() const {
    std::vector<RuntimeType> runtimeTypes;
    for (auto it{ Iterator(std::to_address(mData.begin())) };
        it != std::to_address(mData.end()); ++it) {
        runtimeTypes.emplace_back(Iterator(std::to_address(it)).toRuntimeType());
    }
    return runtimeTypes;
}

/////////////////////////////// RuntimeTypeLookUp::Iterator ////////////////////

constexpr RuntimeTypeLookUp::Iterator::Iterator(pointer ptr) : mPtr{ ptr } {}

constexpr RuntimeTypeLookUp::Iterator::reference
RuntimeTypeLookUp::Iterator::operator*() const {
    return *mPtr;
}

constexpr RuntimeTypeLookUp::Iterator::pointer
RuntimeTypeLookUp::Iterator::operator->() const {
    return mPtr;
}

constexpr RuntimeTypeLookUp::Iterator&
RuntimeTypeLookUp::Iterator::operator++() {
    mPtr += mPtr->getJumpEnd();
    return *this;
}

constexpr RuntimeTypeLookUp::Iterator
RuntimeTypeLookUp::Iterator::operator++(int) {
    Iterator temp = *this;
    ++(*this);
    return temp;
}

constexpr RuntimeTypeLookUp::Iterator::pointer
RuntimeTypeLookUp::Iterator::get() const {
    return mPtr;
}

constexpr bool
RuntimeTypeLookUp::Iterator::operator==(const Iterator& other) const {
    return mPtr == other.mPtr;
}

constexpr bool
RuntimeTypeLookUp::Iterator::operator!=(const Iterator& other) const {
    return mPtr != other.mPtr;
}

inline RuntimeTypeLookUp RuntimeTypeLookUp::Iterator::lookUp() const {
    if (mPtr->isStorableType()) {
        return RuntimeTypeLookUp(mPtr, mPtr->getJumpEnd());
    }

    return RuntimeTypeLookUp(mPtr, 1);
}

inline RuntimeTypeLookUp RuntimeTypeLookUp::Iterator::lookUpChildren() const {
    assert(mPtr->isStorableType() && mPtr->getJumpEnd() > 0);
    return RuntimeTypeLookUp(mPtr + 1, mPtr->getJumpEnd() - 1);
}

inline RuntimeType RuntimeTypeLookUp::Iterator::toRuntimeType() const {
    if (mPtr->isStorableType()) {
        auto sptr{
            std::make_shared<BitFieldRuntimeCompoundType[]>(mPtr->getJumpEnd()) };
        for (size_t ind{ 0 }; ind < mPtr->getJumpEnd(); ++ind) {
            sptr[ind] = mPtr[ind];
        }
        return RuntimeType(
            SharedSpan<BitFieldRuntimeCompoundType>(sptr, mPtr->getJumpEnd()));
    }

    return RuntimeType(*mPtr);
}

/////////////////////////////// HeavyRuntimeTypeLookUp /////////////////////////

inline HeavyRuntimeTypeLookUp::HeavyRuntimeTypeLookUp(
    const RuntimeType& runtimeType)
    : RuntimeTypeLookUp(runtimeType.getRawType()),
    mSharedPtr{ runtimeType.mType.getShared() } {}

inline HeavyRuntimeTypeLookUp::HeavyRuntimeTypeLookUp(
    std::shared_ptr<BitFieldRuntimeCompoundType[]> sharedPtr,
    Iterator::pointer begin, size_t count)
    : RuntimeTypeLookUp(begin, count), mSharedPtr{ sharedPtr } {}

inline std::vector<RuntimeType> HeavyRuntimeTypeLookUp::toRuntimeTypes() const {
    std::vector<RuntimeType> runtimeTypes;
    for (auto it{ Iterator(*this, std::to_address(mData.begin())) };
        it != std::to_address(mData.end()); ++it) {
        runtimeTypes.emplace_back(Iterator(*this, std::to_address(it)).toRuntimeType());
    }
    return runtimeTypes;
}

constexpr HeavyRuntimeTypeLookUp::Iterator
HeavyRuntimeTypeLookUp::begin() const {
    return Iterator(*this, std::to_address(mData.begin()));
}

constexpr HeavyRuntimeTypeLookUp::Iterator HeavyRuntimeTypeLookUp::end() const {
    return Iterator(*this, std::to_address(mData.end()));
    return Iterator(*this, std::to_address(mData.end()));
}

constexpr HeavyRuntimeTypeLookUp::Iterator::Iterator(
    const HeavyRuntimeTypeLookUp& outterClass, pointer ptr)
    : RuntimeTypeLookUp::Iterator(ptr), outer{ outterClass } {}

inline RuntimeType HeavyRuntimeTypeLookUp::Iterator::toRuntimeType() const {
    return RuntimeType(SharedSpan<BitFieldRuntimeCompoundType>(
        outer.mSharedPtr, mPtr - outer.mSharedPtr.get(), mPtr->getJumpEnd()));
}

inline HeavyRuntimeTypeLookUp
HeavyRuntimeTypeLookUp::Iterator::heavyLookUp() const {
    if (mPtr->isStorableType()) {
        return HeavyRuntimeTypeLookUp(outer.mSharedPtr, mPtr, mPtr->getJumpEnd());
    }

    return HeavyRuntimeTypeLookUp(outer.mSharedPtr, mPtr, 1);
}

inline HeavyRuntimeTypeLookUp
HeavyRuntimeTypeLookUp::Iterator::heavyLookUpChildren() const {
    assert(mPtr->isStorableType() && mPtr->getJumpEnd() > 0);
    return HeavyRuntimeTypeLookUp(outer.mSharedPtr, mPtr + 1,
        mPtr->getJumpEnd() - 1);
}

/////////////////////////////// LambdaTypeLookUp ///////////////////////////////

inline LambdaTypeLookUp::LambdaTypeLookUp(const RuntimeType& runtimeType)
    : HeavyRuntimeTypeLookUp(runtimeType) {
    assert(runtimeType.getBaseType() == RuntimeBaseType::_Lambda);
}

inline LambdaTypeLookUp::LambdaTypeLookUp(
    const HeavyRuntimeTypeLookUp& heavyLookUp)
    : HeavyRuntimeTypeLookUp(heavyLookUp) {
    assert(heavyLookUp.begin()->getBaseRuntimeType() == RuntimeBaseType::_Lambda);
}

inline RuntimeType LambdaTypeLookUp::returnType() const {
    return Iterator(*this, begin().get() + 1).toRuntimeType();
}

inline RuntimeType LambdaTypeLookUp::paramsType() const {
    return std::next(Iterator(*this, begin().get() + 1)).toRuntimeType();
}

// Not a lazy function, calculate params size on called.
inline size_t LambdaTypeLookUp::paramsSize() const {
    if (lookUpParamsType().begin()->getBaseRuntimeType() != RuntimeBaseType::_Storage)
        return 1;
    size_t size{ 0 };
    RuntimeTypeLookUp childrens{ lookUpParamsType().begin().lookUpChildren() };
    for (auto it{ childrens.begin() }; it != childrens.end(); ++it, ++size);
    return size;
}

inline RuntimeTypeLookUp LambdaTypeLookUp::lookUpReturnType() const {
    return Iterator(*this, begin().get() + 1).lookUp();
}

inline RuntimeTypeLookUp LambdaTypeLookUp::lookUpParamsType() const {
    return std::next(Iterator(*this, begin().get() + 1)).lookUp();
}

inline HeavyRuntimeTypeLookUp LambdaTypeLookUp::heavyLookUpReturnType() const {
    return Iterator(*this, begin().get() + 1).heavyLookUp();
}

inline HeavyRuntimeTypeLookUp LambdaTypeLookUp::heavyLookUpParamsType() const {
    return std::next(Iterator(*this, begin().get() + 1)).heavyLookUp();
}

/////////////////////////////// StorageRuntimeType /////////////////////////////

inline StorageTypeLookUp::StorageTypeLookUp(const RuntimeType& runtimeType)
    : HeavyRuntimeTypeLookUp(runtimeType) {
    assert(runtimeType.getBaseType() == RuntimeBaseType::_Storage);
    mSize = get_initailize_size(*this, Iterator(*this, begin().get() + 1));
}

inline StorageTypeLookUp::StorageTypeLookUp(
    const HeavyRuntimeTypeLookUp& heavyLookUp)
    : HeavyRuntimeTypeLookUp(heavyLookUp) {
    assert(heavyLookUp.begin()->getBaseRuntimeType() ==
        RuntimeBaseType::_Storage);
    mSize = get_initailize_size(*this, Iterator(*this, begin().get() + 1));
}

inline RuntimeType StorageTypeLookUp::at(size_t index) const {
    assert(index < size());
    auto it{ Iterator(*this, begin().get() + 1) };
    std::advance(it, index);
    return it.toRuntimeType();
}

inline RuntimeTypeLookUp StorageTypeLookUp::lookUpAt(size_t index) const {
    assert(index < size());
    auto it{ Iterator(*this, begin().get() + 1) };
    std::advance(it, index);
    return it.lookUp();
}

inline HeavyRuntimeTypeLookUp
StorageTypeLookUp::heavyLookUpAt(size_t index) const {
    assert(index < size());
    auto it{ Iterator(*this, begin().get() + 1) };
    std::advance(it, index);
    return it.heavyLookUp();
}

inline size_t StorageTypeLookUp::size() const { return mSize; }

// This method isn't cheap, use size() to get initailized size
inline size_t
StorageTypeLookUp::get_initailize_size(HeavyRuntimeTypeLookUp& outer,
    Iterator firstChildrenIterator) const {
    size_t count{ 0 };
    for (auto it{ Iterator(*this, begin().get() + 1) }; it != end(); ++it, ++count)
        ;
    return count;
}

/////////////////////////////// RuntimeType ////////////////////////////////////

// implements RuntimeType constructor
inline RuntimeType::RuntimeType(
    const SharedSpan<BitFieldRuntimeCompoundType>& type)
    : mType{ type } {}

inline RuntimeType::RuntimeType(SharedSpan<BitFieldRuntimeCompoundType>&& type)
    : mType{ std::move(type) } {}

inline RuntimeType::RuntimeType(const BitFieldRuntimeCompoundType& type)
    : mType{ std::make_shared<BitFieldRuntimeCompoundType[]>(1), 1 } {
    mType.getSpan()[0] = type;
}

inline RuntimeType::RuntimeType()
    : mType{ std::make_shared<BitFieldRuntimeCompoundType[]>(1), 1 } {
    mType.getSpan()[0] =
        BitFieldRuntimeCompoundType(0, RuntimeBaseType::Number, 1);
}

inline RuntimeType::RuntimeType(RuntimeBaseType type)
    : mType{ std::make_shared<BitFieldRuntimeCompoundType[]>(1), 1 } {
    mType.getSpan()[0] =
        BitFieldRuntimeCompoundType(static_cast<uint8_t>(type) >= 2, type, 1);
}

// implements undirect constructor for Storage RuntimeType
inline RuntimeType RuntimeType::gurantreeNoRuntimeEvaluateStorage(
    const std::vector<RuntimeType>& base) {
    return RuntimeType(convertToRuntimeType(RuntimeBaseType::_Storage, base));
}

template <RuntimeTypedRequired... Args>
inline RuntimeType RuntimeType::Storage(Args &&...base) {
    constexpr size_t count = sizeof...(base);
    std::vector<RuntimeType> tmp;
    tmp.reserve(count);
    (tmp.emplace_back(std::move(std::forward<Args>(base))), ...);
    return RuntimeType(convertToRuntimeType(RuntimeBaseType::_Storage, tmp));
}

inline RuntimeType RuntimeType::Storage(const std::vector<RuntimeType>& base) {
    for (const RuntimeType& element : base) {
        assert(static_cast<int8_t>(element.getBaseType()) <= 3);
    }

    return RuntimeType(convertToRuntimeType(RuntimeBaseType::_Storage, base));
}

// implements undirect constructor for Lambda RuntimeType
inline RuntimeType RuntimeType::Lambda(const RuntimeType& Ret,
    const RuntimeType& Params) {
    return RuntimeType(
        convertToRuntimeType(RuntimeBaseType::_Lambda, { Ret, Params }));
}

inline RuntimeType RuntimeType::Lambda(RuntimeType&& Ret,
    RuntimeType&& Params) {
    std::vector<RuntimeType> tmp;
    tmp.reserve(2);
    tmp.emplace_back(Ret);
    tmp.emplace_back(Params);
    return RuntimeType(convertToRuntimeType(RuntimeBaseType::_Lambda, tmp));
}

// implements undirect constructors for Operator Notation Lambda RuntimeType
inline RuntimeType
RuntimeType::HiddenType::_Operator_Lambda_Infix(const RuntimeType& Ret) {
    return RuntimeType(convertToRuntimeType(
        RuntimeBaseType::_Operator_Lambda_Infix, { Ret, RuntimeType::HiddenType::_Stroage_Any }));
}

inline RuntimeType RuntimeType::HiddenType::_Operator_Lambda_Infix(RuntimeType&& Ret) {
    std::vector<RuntimeType> tmp;
    tmp.reserve(2);
    tmp.emplace_back(Ret);
    tmp.emplace_back(RuntimeType::HiddenType::_Stroage_Any);
    return RuntimeType(
        convertToRuntimeType(RuntimeBaseType::_Operator_Lambda_Infix, tmp));
}

inline RuntimeType
RuntimeType::HiddenType::_Operator_Lambda_Postfix(const RuntimeType& Ret) {
    return RuntimeType(convertToRuntimeType(
        RuntimeBaseType::_Operator_Lambda_Postfix, { Ret, RuntimeType::HiddenType::_Stroage_Any }));
}

inline RuntimeType RuntimeType::HiddenType::_Operator_Lambda_Postfix(RuntimeType&& Ret) {
    std::vector<RuntimeType> tmp;
    tmp.reserve(2);
    tmp.emplace_back(Ret);
    tmp.emplace_back(RuntimeType::HiddenType::_Stroage_Any);
    return RuntimeType(
        convertToRuntimeType(RuntimeBaseType::_Operator_Lambda_Postfix, tmp));
}

inline RuntimeType
RuntimeType::HiddenType::_Operator_Lambda_Prefix(const RuntimeType& Ret) {
    return RuntimeType(convertToRuntimeType(
        RuntimeBaseType::_Operator_Lambda_Prefix, { Ret, RuntimeType::HiddenType::_Stroage_Any }));
}

inline RuntimeType RuntimeType::HiddenType::_Operator_Lambda_Prefix(RuntimeType&& Ret) {
    std::vector<RuntimeType> tmp;
    tmp.reserve(2);
    tmp.emplace_back(Ret);
    tmp.emplace_back(RuntimeType::HiddenType::_Stroage_Any);
    return RuntimeType(
        convertToRuntimeType(RuntimeBaseType::_Operator_Lambda_Prefix, tmp));
}

inline RuntimeType
RuntimeType::HiddenType::_Operator_Lambda_Constant(const RuntimeType& Ret) {
    return RuntimeType(convertToRuntimeType(
        RuntimeBaseType::_Operator_Lambda_Constant, { Ret, RuntimeType::HiddenType::_Stroage_Any }));
}

inline RuntimeType
RuntimeType::HiddenType::_Operator_Lambda_Constant(RuntimeType&& Ret) {
    std::vector<RuntimeType> tmp;
    tmp.reserve(2);
    tmp.emplace_back(Ret);
    tmp.emplace_back(RuntimeType::HiddenType::_Stroage_Any);
    return RuntimeType(
        convertToRuntimeType(RuntimeBaseType::_Operator_Lambda_Constant, tmp));
}

// implements function for converting to unify RuntimeType
inline SharedSpan<BitFieldRuntimeCompoundType>
RuntimeType::convertToRuntimeType(
    RuntimeBaseType base,
    std::vector<RuntimeType>::const_iterator children_begin,
    std::vector<RuntimeType>::const_iterator children_end) {

    // calculate total size need to be allocate for new RuntimeType
    size_t totalTypeSize{ 0 };
    for (auto it{ children_begin }; it != children_end; it++) {
        totalTypeSize += it->getRawType().size();
    }

    // allocate memory for RuntimeType
    std::shared_ptr<BitFieldRuntimeCompoundType[]> compoundType{
        std::make_shared<BitFieldRuntimeCompoundType[]>(
            totalTypeSize +
            1) }; // totalTypeSize + 1 becasue need to account for the baseType.

    // set first index of RuntimeType to be a Storage
    compoundType[0] = BitFieldRuntimeCompoundType(true, base, totalTypeSize + 1);
    // perform a deep copy
    size_t ind{ 1 };
    for (auto it{ children_begin }; it != children_end; it++) {
        for (const auto& spanBitField : it->getRawType()) {
            compoundType[ind++] = spanBitField;
        }
    }

    return SharedSpan<BitFieldRuntimeCompoundType>(compoundType,
        totalTypeSize + 1);
}

inline SharedSpan<BitFieldRuntimeCompoundType>
RuntimeType::convertToRuntimeType(const RuntimeType& base,
    const std::vector<RuntimeType>& children) {
    assert(children.size() >= 1 && static_cast<uint8_t>(base.getBaseType()) >= 2);
    return convertToRuntimeType(base.getBaseType(), children.begin(),
        children.end());
}

inline SharedSpan<BitFieldRuntimeCompoundType>
RuntimeType::convertToRuntimeType(RuntimeBaseType base,
    const std::vector<RuntimeType>& children) {
    assert(children.size() >= 1 && static_cast<uint8_t>(base) >= 2);
    return convertToRuntimeType(base, children.begin(), children.end());
}

inline SharedSpan<BitFieldRuntimeCompoundType>
RuntimeType::convertToRuntimeType(const std::vector<RuntimeType>& fullType) {
    assert(fullType.size() >= 2 &&
        static_cast<uint8_t>(fullType.front().getBaseType()) >= 2);
    return convertToRuntimeType(fullType.front().getBaseType(),
        std::next(fullType.begin()), fullType.end());
}

inline RuntimeBaseType RuntimeType::getBaseType() const {
    return mType.getSpan().front().getBaseRuntimeType();
}

inline const std::span<const BitFieldRuntimeCompoundType>
RuntimeType::getRawType() const {
    return mType.getSpan();
}

// Recommended way to get a literal child from the type.
inline std::vector<RuntimeType> RuntimeType::getChildren() const {
    assert(mType.getSpan().front().isStorableType() &&
        mType.getSpan().front().getJumpEnd());
    std::vector<RuntimeType> children;
    RuntimeTypeLookUp lookup(getRawType());

    for (auto it{ lookup.begin() }; it != lookup.end(); ++it) {
        children.emplace_back(SharedSpan<BitFieldRuntimeCompoundType>(
            mType.getShared(), it.get() - lookup.begin().get(), it->getJumpEnd()));
    }
    return children;
}

inline RuntimeTypeLookUp RuntimeType::lookUpChildren() const {
    assert(getRawType().front().isStorableType() &&
        getRawType().front().getJumpEnd());
    return RuntimeTypeLookUp(std::to_address(getRawType().begin()) + 1,
        mType.getSpan().front().getJumpEnd() - 1);
}

inline HeavyRuntimeTypeLookUp RuntimeType::heavyLookUpChildren() const {
    assert(getRawType().front().isStorableType() &&
        getRawType().front().getJumpEnd());
    return HeavyRuntimeTypeLookUp(mType.getShared(),
        std::to_address(getRawType().begin()) + 1,
        getRawType().front().getJumpEnd() - 1);
}

inline LambdaTypeLookUp RuntimeType::asLambda() const {
    return LambdaTypeLookUp(*this);
}

inline StorageTypeLookUp RuntimeType::asStorage() const {
    return StorageTypeLookUp(*this);
}

inline std::string RuntimeType::toString() const {
    std::stringstream stringifyType;
    std::stack<uint32_t> closeParenthesis;
    bool shouldNotPutSep{ true };

    for (uint32_t ind{ 0 }; ind < mType.getSpan().size(); ++ind) {
        if (closeParenthesis.size() && closeParenthesis.top() == ind) {
            stringifyType << "]";
            closeParenthesis.pop();
        }

        if (!shouldNotPutSep)
            stringifyType << ", ";
        else
            shouldNotPutSep = false;

        stringifyType << runtimeBaseTypeToString(
            mType.getSpan()[ind].getBaseRuntimeType());

        if (mType.getSpan()[ind].isStorableType()) {
            shouldNotPutSep = true;
            stringifyType << "[";
            closeParenthesis.push(mType.getSpan()[ind].getJumpEnd() + ind);
        }
    }

    while (closeParenthesis.size()) {
        stringifyType << "]";
        closeParenthesis.pop();
    }

    return stringifyType.str();
}

inline Result<RuntimeType, std::runtime_error>
RuntimeType::ParseString(const std::string& stringLikeType) {
    // Initialize the lexer if not already done
    static Lexer lex;
    static bool initialized = false;

    if (!initialized)
        lex.setKeywords({ "Number", "NodePointer", "Storage", "Lambda", "[", "]" });

    // Tokenize the input string using the lexer
    std::vector<std::string> lexemes{ lex.lexing(stringLikeType).getValue() };
    std::stack<RuntimeType> operationStack;

    // ***Open bracket ("[") is interpreted as RuntimeType::_Stroage_Any
    // Some operations use RuntimeBaseType for comparisons because it fits the use
    // case and is much faster.

    // Process each lexeme
    for (const std::string& lexeme : lexemes) {
        // Handle closing bracket
        if (lexeme == "]") {
            // Collect elements until an opening bracket is found
            std::vector<RuntimeType> containRuntimeType;
            while (!operationStack.empty() && operationStack.top().getBaseType() !=
                RuntimeBaseType::_Stroage_Any) {
                containRuntimeType.emplace_back(operationStack.top());
                operationStack.pop();
            }
            if (operationStack.size() < 2)
                return std::runtime_error("");

            // Pop the opening bracket
            operationStack.pop();

            // Check the type of the element before the opening bracket
            if (operationStack.top().getBaseType() != RuntimeBaseType::_Lambda &&
                operationStack.top().getBaseType() != RuntimeBaseType::_Storage)
                return RuntimeError<RuntimeTypeError>(
                    "Holder of RuntimeTypes must be RuntimeBaseType::_Lambda or "
                    "RuntimeBaseType::_Storage.",
                    "RuntimeCompoundType::ParseString");

            // Determine the type of the element and create the corresponding
            // RuntimeCompoundType
            RuntimeBaseType runtimeTypeElementHolder{
                operationStack.top().getBaseType() };
            operationStack.pop();

            std::reverse(containRuntimeType.begin(), containRuntimeType.end());
            if (runtimeTypeElementHolder == RuntimeBaseType::_Storage)
                operationStack.emplace(
                    RuntimeType::gurantreeNoRuntimeEvaluateStorage(containRuntimeType));

            else if (runtimeTypeElementHolder == RuntimeBaseType::_Lambda &&
                containRuntimeType.size() == 2)
                operationStack.emplace(
                    RuntimeType::Lambda(containRuntimeType[0], containRuntimeType[1]));
            else
                return RuntimeError<RuntimeTypeError>(
                    "RuntimeBaseType::_Lambda can hold only 2 argument (including "
                    "Return and Params), use Storage if you want to return "
                    "compoundType.",
                    "RuntimeCompoundType::ParseString");
        }

        // Handle opening bracket
        else if (lexeme == "[")
            operationStack.emplace(RuntimeType::HiddenType::_Stroage_Any);

        // Handle other types
        else {
            if (lexeme == "Number")
                operationStack.emplace(RuntimeType::Number);
            else if (lexeme == "NodePointer")
                operationStack.emplace(RuntimeType::NodePointer);
            else if (lexeme == "Storage")
                operationStack.emplace(RuntimeType::HiddenType::_Storage);
            else
                operationStack.emplace(RuntimeType::HiddenType::_Lambda);
        }
    }

    // Check the final state of the stack
    if (operationStack.size() != 1 ||
        operationStack.top().getBaseType() == RuntimeBaseType::_Stroage_Any)
        return RuntimeError<RuntimeTypeError>(
            "Failed to evaluate RuntimeType like string.",
            "RuntimeCompoundType::ParseString");

    return operationStack.top();
}

inline bool RuntimeType::operator==(const RuntimeType& other) const {
    if (mType.getSpan().size() != other.mType.getSpan().size())
        return false;

    for (size_t ind{ 0 }; ind < mType.getSpan().size(); ++ind) {
        if (mType.getSpan()[ind] != other.mType.getSpan()[ind])
            return false;
    }

    return true;
}

inline bool RuntimeType::operator!=(const RuntimeType& other) const {
    return !operator==(other);
}

inline std::ostream& operator<<(std::ostream& os,
    const RuntimeType& runtimeType) {
    os << runtimeType.toString();
    return os;
}

// Formatter for RuntimeType
template <>
struct std::formatter<RuntimeType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const RuntimeType& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", obj.toString());
    }
};
#endif // !TYPE_DEMO_IMPL
