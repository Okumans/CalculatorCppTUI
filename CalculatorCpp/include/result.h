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
    bool mIsError;

public:
    Result(const T& value) : mValueOrException(value), mIsError(false) {}
    Result(const E& exception) : mValueOrException(exception), mIsError(true) {}
    Result(const Result& other) : mValueOrException(other.mValueOrException), mIsError(other.mIsError) {}

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
