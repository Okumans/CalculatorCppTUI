#ifndef RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY
#define RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY

#include "runtimeTypedExprComponent.h"
#include <format>

inline const Number& RuntimeTypedExprComponent::getNumber() const {
    return std::get<Number>(*this);
}

inline const Lambda& RuntimeTypedExprComponent::getLambda() const {
    return std::get<Lambda>(*this);
}

inline const Storage& RuntimeTypedExprComponent::getStorage() const {
    return std::get<Storage>(*this);
}

inline RuntimeBaseType RuntimeTypedExprComponent::getTypeHolded() const {
    return mStoredType;
}

inline RuntimeType RuntimeTypedExprComponent::getDetailTypeHold() const {
    return std::visit([](const auto& arg) -> RuntimeType {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, Number> ||
            std::is_same_v<std::decay_t<decltype(arg)>, Storage> ||
            std::is_same_v<std::decay_t<decltype(arg)>, Lambda>) {
            return arg.getType();
        }
        throw std::runtime_error("Invalid type in variant.");
        }, *this);
}

inline const RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator[](size_t index) const {
    if (getTypeHolded() != RuntimeBaseType::_Storage)
        throw std::runtime_error("Unable to access an element from a non-Storage type.");
    return getStorage()[index];
}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Storage& component) :
    mStoredType{ RuntimeBaseType::_Storage },
    std::variant<Number, Storage, Lambda>(component) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Storage&& component) :
    mStoredType{ RuntimeBaseType::_Storage },
    std::variant<Number, Storage, Lambda>(std::move(component)) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Lambda& component) :
    mStoredType{ RuntimeBaseType::_Lambda },
    std::variant<Number, Storage, Lambda>(component) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Lambda&& component) :
    mStoredType{ RuntimeBaseType::_Lambda },
    std::variant<Number, Storage, Lambda>(std::move(component)) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const Number& component) :
    mStoredType{ RuntimeBaseType::Number },
    std::variant<Number, Storage, Lambda>(component) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(Number&& component) :
    mStoredType{ RuntimeBaseType::Number },
    std::variant<Number, Storage, Lambda>(std::move(component)) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(long double component) :
    mStoredType{ RuntimeBaseType::Number },
    std::variant<Number, Storage, Lambda>(Number(component)) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(const RuntimeTypedExprComponent& other) :
    mStoredType{ other.mStoredType },
    std::variant<Number, Storage, Lambda>(other) {}

inline RuntimeTypedExprComponent::RuntimeTypedExprComponent(RuntimeTypedExprComponent&& other) noexcept :
    mStoredType{ std::move(other.mStoredType) },
    std::variant<Number, Storage, Lambda>(std::move(other)) {}

inline RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator=(const RuntimeTypedExprComponent& other) {
    if (this != &other) {
        mStoredType = other.mStoredType;
        std::variant<Number, Storage, Lambda>::operator=(other);
    }
    return *this;
}

inline RuntimeTypedExprComponent& RuntimeTypedExprComponent::operator=(RuntimeTypedExprComponent&& other) noexcept {
    if (this != &other) {
        mStoredType = std::move(other.mStoredType);
        std::variant<Number, Storage, Lambda>::operator=(std::move(other));
    }
    return *this;
}

inline std::string RuntimeTypedExprComponent::toString() const {
    return std::visit([](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Number> ||
            std::is_same_v<T, Storage> ||
            std::is_same_v<T, Lambda>) {
            return arg.toString();
        }
        throw std::runtime_error("Invalid type in variant.");
        }, *this);
}

inline NodeFactory::NodePos RuntimeTypedExprComponent::toNodeExpression() const {
    return std::visit([](const auto& arg) -> NodeFactory::NodePos {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Number> ||
            std::is_same_v<T, Storage> ||
            std::is_same_v<T, Lambda>) {
            return arg.getNodeExpression();
        }
        throw std::runtime_error("Invalid type in variant. Btw how can it happen?");
        }, *this);
}

inline Result<RuntimeTypedExprComponent, std::runtime_error> RuntimeTypedExprComponent::fromNodeExpression(NodeFactory::NodePos rootNodeExpression, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions) {
    NodeFactory::Node::NodeState currNodeState = NodeFactory::node(rootNodeExpression).nodestate;
    switch (currNodeState)
    {
    case NodeFactory::Node::NodeState::LambdaFuntion:
    {
        Result result = Lambda::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions);
        if (result.isError())
            return RuntimeTypeError(
                result.getException(),
                std::format("When trying to convert nodeExpression (value=\"{}\") to LambdaFunction.",
                    NodeFactory::node(rootNodeExpression).value),
                "RuntimeTypedExprComponent::fromNodeExpression"
            );

        return RuntimeTypedExprComponent(result.getValue());
    }
    case NodeFactory::Node::NodeState::Operator:
    {
        Result result = Lambda::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions);
        if (result.isError())
            return RuntimeTypeError(
                result.getException(),
                std::format(
                    "When trying to convert nodeExpression (value=\"{}\") to Operator.",
                    NodeFactory::node(rootNodeExpression).value
                ),
                "RuntimeTypedExprComponent::fromNodeExpression"
            );

        return RuntimeTypedExprComponent(result.getValue());
    }
    case NodeFactory::Node::NodeState::Number:
        return RuntimeTypedExprComponent(Number::fromExpressionNode(rootNodeExpression));

    case NodeFactory::Node::NodeState::Storage:
    {
        Result result = Storage::fromExpressionNode(rootNodeExpression, EvaluatorLambdaFunctions);
        if (result.isError())
            return RuntimeTypeError(
                result.getException(),
                std::format(
                    "When trying to convert nodeExpression (value=\"{}\") to Storage.",
                    NodeFactory::node(rootNodeExpression).value
                ),
                "RuntimeTypedExprComponent::fromNodeExpression"
            );

        return RuntimeTypedExprComponent(result.getValue());
    }
    }
    return RuntimeTypeError(
        "The conversion from nodeExpression to LambdaFunction failed due to an invalid nodeState.",
        "RuntimeTypedExprComponent::fromNodeExpression");
}

inline std::ostream& operator<<(std::ostream& os, const RuntimeTypedExprComponent& rttexcp) {
    os << rttexcp.toString();
    return os;
}

inline std::vector<std::string_view> splitString(std::string_view in, char sep) {
    std::vector<std::string_view> r;
    r.reserve(std::count(in.begin(), in.end(), sep) + 1); // optional
    for (auto p = in.begin();; ++p) {
        auto q = p;
        p = std::find(p, in.end(), sep);
        r.emplace_back(q, p);
        if (p == in.end())
            return r;
    }
}

inline bool _fastCheckRuntimeTypeArgumentsType(const RuntimeType& baseType, const std::vector<RuntimeTypedExprComponent>& argumentsCheckType) {
    if (!std::holds_alternative<RuntimeCompoundType>(baseType) ||
        std::get<RuntimeCompoundType>(baseType).Type != RuntimeBaseType::_Storage ||
        RuntimeCompoundType::getStorageInfo(baseType).StorageSize != argumentsCheckType.size())
        return false;

    const auto& argumentsBaseType = RuntimeCompoundType::getStorageInfo(baseType);

    for (size_t ind{ 0 }, len{ argumentsCheckType.size() }; ind < len; ind++) {
        const auto a = argumentsBaseType.Storage->at(ind);
        if (std::holds_alternative<Number>(argumentsCheckType[ind]) && argumentsBaseType.Storage->at(ind) != RuntimeBaseType::Number)
            return false;
        if (std::holds_alternative<Lambda>(argumentsCheckType[ind]) && std::get<Lambda>(argumentsCheckType[ind]).getType() != argumentsBaseType.Storage->at(ind))
            return false;
        if (std::holds_alternative<Storage>(argumentsCheckType[ind]) && std::get<Storage>(argumentsCheckType[ind]).getType() != argumentsBaseType.Storage->at(ind))
            return false;
    }
    return true;
}

inline Result<RuntimeType, std::runtime_error> getReturnType(NodeFactory::NodePos rootExpressionNode, const std::unordered_map<std::string, Lambda>& EvaluatorLambdaFunctions, bool useCache) {
    static std::unordered_map<NodeFactory::NodePos, RuntimeType> nodeTypeCache;

    if (useCache && nodeTypeCache.contains(rootExpressionNode))
        return nodeTypeCache[rootExpressionNode];

    std::stack<NodeFactory::NodePos> operationStack;
    std::unordered_map<NodeFactory::NodePos, RuntimeType> resultMap;

    operationStack.push(rootExpressionNode);
    while (!operationStack.empty()) {
        const NodeFactory::NodePos currNodePos{ operationStack.top() };

        if (!NodeFactory::validNode(currNodePos)) {
            operationStack.pop();
            continue;
        }

        const NodeFactory::Node currNode{ NodeFactory::node(currNodePos) }; // cannot gurantree node to not change value, need to be copy.

        // if currNode is a leaf node.
        if (!NodeFactory::validNode(currNode.rightPos) &&
            !NodeFactory::validNode(currNode.leftPos)) {
            if (currNode.value == ".")
                resultMap[currNodePos] = RuntimeBaseType::Number;

            else if (EvaluatorLambdaFunctions.contains(currNode.value) &&
                EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Constant) {
                resultMap[currNodePos] = *EvaluatorLambdaFunctions.at(currNode.value).getLambdaInfo().ReturnType;
            }
            else
                resultMap[currNodePos] = RuntimeBaseType::Number;
        }

        else if (currNode.nodestate == NodeFactory::Node::NodeState::LambdaFuntion) {
            std::vector<std::pair<std::string, RuntimeType>> parameters{ currNode.utilityStorage };
            std::vector<RuntimeType> arguments;

            NodeFactory::NodePos currArgNodePos{ currNode.rightPos };
            while (NodeFactory::validNode(currArgNodePos)) {
                Result<RuntimeType, std::runtime_error> result{ getReturnType(NodeFactory::node(currArgNodePos).leftPos, EvaluatorLambdaFunctions) };

                // Handle error occur from determining argument type 
                if (result.isError())
                    return RuntimeTypeError(
                        result.getException(),
                        std::format(
                            "While determining argument type of \"{}\"",
                            NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
                            ? NodeFactory::node(currArgNodePos).leftNode().value
                            : "Null"
                        ),
                        "getReturnType"
                    );

                arguments.push_back(result.getValue());
                currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
            }

            if (arguments.size() && parameters.size() && arguments.size() != parameters.size())
                return RuntimeTypeError(std::format("parameters size must be equal to argument size! ({}!={})", parameters.size(), arguments.size()), "getReturnType");

            std::unordered_map<std::string, Lambda> tempEvaluatorLambdaFunctions(EvaluatorLambdaFunctions);
            std::vector<RuntimeType> lambdaParameterType;
            lambdaParameterType.reserve(parameters.size());

            // depend on arguments size in this case the argument size should be either same as parameter or 0.
            for (size_t ind{ 0 }; ind < parameters.size(); ind++) {
                RuntimeType& parameterType{ parameters[ind].second };
                
                if (arguments.size() && parameterType != arguments[ind])
                    return RuntimeTypeError(std::format("parameters type must be same as argument type! ({}!={})", parameterType, arguments[ind]), "getReturnType");

                Result<Lambda, std::runtime_error> tempFunc{ Lambda::fromFunction(
                    parameters[ind].first,
                    RuntimeCompoundType::Lambda(
                        parameterType,
                        RuntimeBaseType::_Storage
                    ),
                    Lambda::LambdaNotation::Constant,
                    [](const Lambda::LambdaArguments&) {
                        return NodeFactory::NodePosNull;
                    }
                ) };

                if (tempFunc.isError())
                    return LambdaConstructionError(
                        tempFunc.getException(),
                        std::format(
                            "When attempting to create a constant lambda function \"{}\", which is used to assist in defining the return type",
                            parameters[ind].first
                        ),
                        "getReturnType"
                    );

                tempEvaluatorLambdaFunctions.try_emplace(parameters[ind].first, tempFunc.moveValue());
                lambdaParameterType.emplace_back(parameters[ind].second);
            }

            Result<RuntimeType, std::runtime_error> leftVal{ getReturnType(currNode.leftPos, tempEvaluatorLambdaFunctions) };

            if (leftVal.isError())
                return RuntimeTypeError(
                    leftVal.getException(),
                    std::format("While determining argument type of \"{}\"",
                        NodeFactory::validNode(currNode.leftPos)
                        ? NodeFactory::node(currNode.leftPos).value
                        : "Null"),
                    "getReturnType");

            if (arguments.empty() && parameters.size()) { // didn't call function
                if (lambdaParameterType.empty())
                    resultMap[currNodePos] = RuntimeCompoundType::Lambda(leftVal.moveValue(), RuntimeBaseType::_Storage);
                else if (lambdaParameterType.size() == 1)
                    resultMap[currNodePos] = RuntimeCompoundType::Lambda(leftVal.moveValue(), std::move(lambdaParameterType.front()));
                else
                    resultMap[currNodePos] = RuntimeCompoundType::Lambda(leftVal.moveValue(), RuntimeCompoundType::Storage(std::move(lambdaParameterType)));
            }

            else
                resultMap[currNodePos] = leftVal.moveValue();
        }

        else if (currNode.nodestate == NodeFactory::Node::NodeState::Storage) {
            std::vector<RuntimeType> arguments;

            NodeFactory::NodePos currArgNodePos{ currNodePos };
            while (NodeFactory::validNode(currArgNodePos)) {
                Result<RuntimeType, std::runtime_error> result{ getReturnType(NodeFactory::node(currArgNodePos).leftPos, EvaluatorLambdaFunctions) };

                if (result.isError())
                    return RuntimeTypeError(
                        result.getException(),
                        std::format("While determining argument type of \"{}\"",
                            NodeFactory::validNode(NodeFactory::node(currArgNodePos).leftPos)
                            ? NodeFactory::node(currArgNodePos).leftNode().value
                            : "Null"),
                        "getReturnType");

                arguments.push_back(result.getValue());
                currArgNodePos = NodeFactory::node(currArgNodePos).rightPos;
            }

            resultMap[currNodePos] = RuntimeCompoundType::Storage(arguments);
        }

        else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Infix) {
            if (!resultMap.contains(currNode.leftPos)) {
                operationStack.push(currNode.leftPos);
                continue;
            }

            if (!resultMap.contains(currNode.rightPos)) {
                operationStack.push(currNode.rightPos);
                continue;
            }

            RuntimeType leftType{ resultMap[currNode.leftPos] };
            RuntimeType rightType{ resultMap[currNode.rightPos] };

            // Lambda parameter numbers is guarantree to be 2 (check at Lambda construction.)
            const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };
            const auto& parametersType{ RuntimeCompoundType::getStorageInfo(*lambdaFunction.getLambdaInfo().ParamsType).Storage };
            if (!((*parametersType)[0] == leftType && (*parametersType)[1] == rightType))
                return RuntimeTypeError(
                    std::format("Parameters type must be equal to argument type. ({}!={})",
                        *lambdaFunction.getLambdaInfo().ParamsType,
                        RuntimeType(RuntimeCompoundType::Storage({ leftType, rightType }))),
                    "getReturnType");

            resultMap[currNodePos] = *lambdaFunction.getLambdaInfo().ReturnType;
        }

        else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Postfix) {
            if (!resultMap.contains(currNode.rightPos)) {
                operationStack.push(currNode.rightPos);
                continue;
            }

            RuntimeType rightVal{ resultMap[currNode.rightPos] };
            const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };
            if (*lambdaFunction.getLambdaInfo().ParamsType != rightVal)
                return RuntimeTypeError(
                    std::format("Parameters type must be equal to argument type. ({}!={})",
                        *lambdaFunction.getLambdaInfo().ParamsType,
                        rightVal),
                    "getReturnType");

            resultMap[currNodePos] = *lambdaFunction.getLambdaInfo().ReturnType;

        }

        else if (EvaluatorLambdaFunctions.contains(currNode.value) && EvaluatorLambdaFunctions.at(currNode.value).getNotation() == Lambda::LambdaNotation::Prefix) {
            if (!resultMap.contains(currNode.leftPos)) {
                operationStack.push(currNode.leftPos);
                continue;
            }

            RuntimeType leftVal{ resultMap[currNode.leftPos] };
            const Lambda& lambdaFunction{ EvaluatorLambdaFunctions.at(currNode.value) };
            if (*lambdaFunction.getLambdaInfo().ParamsType == leftVal)
                return RuntimeTypeError(
                    std::format("Parameters type must be equal to argument type. ({}!={})",
                        *lambdaFunction.getLambdaInfo().ParamsType,
                        leftVal),
                    "getReturnType");

            resultMap[currNodePos] = *lambdaFunction.getLambdaInfo().ReturnType;
        }

        else {
            std::cout << currNode.value << std::endl;
        }

        operationStack.pop();
    }

    if (!resultMap.contains(rootExpressionNode))
        return std::runtime_error("failed to find return type.");

    nodeTypeCache[rootExpressionNode] = resultMap[rootExpressionNode];

    return resultMap[rootExpressionNode];
}

#endif //RUNTIME_TYPED_EXPR_COMPONENT_IMPL_UTILITY