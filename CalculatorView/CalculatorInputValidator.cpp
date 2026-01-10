#include "CalculatorInputValidator.h"

#include <QList>

namespace {
const QList<QChar> HEX_SYMBOLS = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

const QList<QChar> OPERATOR_SYMBOLS = {
    '+', '-', '*', '/'
};

bool isHexSymbol(QChar symbol) {
    return HEX_SYMBOLS.contains(symbol);
}

bool isOperatorSymbol(QChar symbol) {
    return OPERATOR_SYMBOLS.contains(symbol);
}
}

CalculatorInputValidator::CalculatorInputValidator() {}

QValidator::State CalculatorInputValidator::validate(QString &input, int &pos) const {
    fixup(input);
    return QValidator::State::Acceptable;
}

QString CalculatorInputValidator::inverseFirst(const QString& input) const {
    QString result = input;

    if (input[0] == '-') {
        result = input.mid(1, input.size());
    }
    else {
        result.push_front('-');
    }

    return result;
}

QString CalculatorInputValidator::appendSymbol(const QString& input, QChar symbol) const {
    QString result = input;

    if (symbol == '.') {
        if (result.back() == '.') {
            return result;
        }

        if (isOperatorSymbol(result.back())) {
            return result;
        }

        for (int i = result.size() - 1; i >= 0; i--) {
            if (result[i] == '.') {
                return result;
            }

            if (isOperatorSymbol(result[i])) {
                break;
            }
        }
    }

    if (isOperatorSymbol(symbol)) {
        if (isOperatorSymbol(result.back())) {
            result.back() = symbol;
            return result;
        }

        if (result.back() == '.') {
            return result;
        }
    }

    if (isHexSymbol(symbol)) {
        if (symbol == '0' && result.back() == '0') {
            if (result.size() == 1) {
                return result;
            }

            for (int i = result.size() - 1; i >= 0; i--) {
                if (result[i] == '.') {
                    result.append(symbol);
                    return result;
                }

                if (isOperatorSymbol(result[i])) {
                    return result;
                }
            }
        }

        if (symbol != '0' && result.back() == '0') {
            if (result.size() == 1) {
                result[0] = symbol;
                return result;
            }
        }
    }

    result.append(symbol);
    return result;
}

QString CalculatorInputValidator::removeSymbol(const QString& input) const {
    QString result = input;

    if (result.size() == 1) {
        result[0] = '0';
        return result;
    }

    if (result.size() == 2 && *result.begin() == '-') {
        result[1] = '0';
        return result;
    }

    result.erase(result.end() - 1);
    return result;
}

void CalculatorInputValidator::fixup(QString& string) const {
    if (string.isEmpty()) {
        string = '0';
    }

    string = string.toUpper();
    string.removeIf([](QChar symbol) {
        return !isHexSymbol(symbol) && !isOperatorSymbol(symbol) && symbol != '.';
    });

    // find invalid continous points
    for (int i = string.size() - 1; i >= 0; i--) {
        if (string[i] == '.') {
            for (int j = i - 1; j >= 0; j--) {
                if (string[j] != '.' && (i - j != 1)) {
                    string.erase(string.begin() + i, string.begin() + j - 1);
                    i = j;
                    break;
                }
            }
        }
    }
}


