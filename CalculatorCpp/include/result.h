#pragma once
#include <exception>
#include <string>
#include <variant>

template <typename T, typename E = std::exception>
class Result {
private:
    std::variant<T, E> mValueOrException;
    bool mIsError;

public:
    Result(const T& value) : mValueOrException(value), mIsError(false) {}
    Result(const E& exception) : mValueOrException(exception), mIsError(true) {}

    bool isError() const {
        return mIsError;
    }

    T getValue() const {
        return std::get<T>(mValueOrException);
    }

    E getException() const {
        return std::get<E>(mValueOrException);
    }

    T getValue_or(T orValue) const {
        return mIsError ? orValue : std::get<T>(mValueOrException);
    }
};
