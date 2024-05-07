#pragma once

#include <exception>
#include <string>
#include <variant>

#define EXCEPT_RETURN(x, ...) \
	do { \
			if (x.isError()) { \
				__VA_ARGS__; \
				return x.getException(); \
			} \
	} while (0)

#define EXCEPT_RETURN_N(x, ...) \
	do { \
			if (!x.isError()) { \
				__VA_ARGS__; \
			} \
	} while (0)

template <typename T, typename E = std::exception>
class Result {
private:
    std::variant<T, E> mValueOrException;

public:
    Result(const T& value) : mValueOrException(value) {}
    Result(T&& value) : mValueOrException(std::move(value)) {}
    Result(const E& exception) : mValueOrException(exception) {}
    Result(E&& exception) : mValueOrException(std::move(exception)) {}
    Result(const Result& other) : mValueOrException(other.mValueOrException) {}
    Result(Result&& other) : mValueOrException(std::move(other.mValueOrException)) {}

    bool isError() const {
        return std::holds_alternative<E>(mValueOrException);
    }

    const T& getValue() const {
        return std::get<T>(mValueOrException);
    }

    const E& getException() const {
        return std::get<E>(mValueOrException);
    }

    const T& getValue_or(const T& orValue) const {
        return isError() ? orValue : std::get<T>(mValueOrException);
    }

    T&& getValue_or(T&& orValue) const {
        return isError() ? std::move(orValue) : std::get<T>(mValueOrException);
    }
};
