#ifndef BIGFLOATMATH_H
#define BIGFLOATMATH_H

enum FixedPointErrorCode {
    NO_ERROR,
    OVERFLOW_ERROR = 1,
    ZERO_DIVISION,
    INVALID_ARGUMENT
};

const char* evaluateExpression(const char* expression, FixedPointErrorCode* errorCode);

#endif // BIGFLOATMATH_H
