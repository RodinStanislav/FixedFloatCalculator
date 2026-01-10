#include "CalculatorBackend.h"

#include "FixedPointFloatMath/FixedPointFloatMath.h"

namespace {
QString getErrorDescription(FixedPointErrorCode code) {
    switch(code) {
        case FixedPointErrorCode::OVERFLOW_ERROR: return "Overflow error";
        case FixedPointErrorCode::ZERO_DIVISION: return "Zero division";
        case FixedPointErrorCode::INVALID_ARGUMENT: return "Invalid argument";
        default: Q_UNREACHABLE();
    }

    return {};
}
}

CalculatorBackend::CalculatorBackend(QObject *parent)
    : ICalculatorBackend{parent}
{}

QString CalculatorBackend::evaluate(const QString& expression) {
    FixedPointErrorCode errorCode = FixedPointErrorCode::NO_ERROR;
    auto evaluatedExpression = evaluateExpression(expression.toUtf8().constData(), &errorCode);

    if (errorCode != 0) {
        auto errorDescription = getErrorDescription(static_cast<FixedPointErrorCode>(errorCode));
        emit errorRaised(errorDescription);
        return {};
    }

    if (evaluatedExpression == nullptr) {
        return {};
    }

    auto result = QString::fromUtf8(evaluatedExpression);
    delete evaluatedExpression;
    return result;
}
