#include "FixedPointFloatMath.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Effictive or potentially bug
// struct HexValue {
//     unsigned char value : 4;
// };

union FixedPointValueData {
    unsigned int parts[4];
    unsigned char bytes[16];
    // HexValue bytes[32];
};

struct FixedPointValue {
    FixedPointValueData value;
    int sign = 1;
};

unsigned char getHexDigitNumber(char symbol) {
    if (symbol <= '9') {
        return symbol - '0';
    }

    return symbol - 'A' + 10;
}

char getHexDigitSymbol(unsigned char number) {
    if (number <= 9) {
        return '0' + number;
    }

    return 'A' + (number - 10);
}

FixedPointErrorCode parseFixedPointValue(const char* string, int size, FixedPointValue* outputValue) {
    memset(outputValue->value.bytes, 0, 16 * sizeof(unsigned char));
    outputValue->sign = 1;

    int pointIndex = -1;

    if (size > 33) { // 32 symbols + 1 to point
        return FixedPointErrorCode::INVALID_ARGUMENT;
    }

    for (int i = 0; i < size; i++) {
        if (string[i] == '\0') {
            break;
        }

        if (string[i] == '.') {
            pointIndex = i;
        }
    }

    if (pointIndex != -1) {
        if (size - pointIndex > 9) { // after point > 8 symbols and point, error
            return FixedPointErrorCode::INVALID_ARGUMENT;
        }

        if (pointIndex > 24) { // 0..23 symbols and point
            return FixedPointErrorCode::INVALID_ARGUMENT;
        }

        if (pointIndex == size - 1) { // 111.?
            return FixedPointErrorCode::INVALID_ARGUMENT;
        }

        if (pointIndex == 0) {
            return FixedPointErrorCode::INVALID_ARGUMENT;
        }
    }
    else {
        if (size > 24) {
            return FixedPointErrorCode::INVALID_ARGUMENT;
        }
    }

    if (pointIndex != -1) {
        int byteIndex = 3;

        for (int i = pointIndex + 1; i < size; i += 2) {
            int value = getHexDigitNumber(string[i]) << 4;
            if (i + 1 < size) {
                value += getHexDigitNumber(string[i + 1]);
            }

            outputValue->value.bytes[byteIndex] = value;
            byteIndex--;
        }
    }

    int startIndex = pointIndex == -1 ? size - 1 : pointIndex - 1;
    int byteIndex = 4;

    for (int i = startIndex; i >= 0; i -= 2) {
        int value = getHexDigitNumber(string[i]);
        if (i - 1 >= 0) {
            value += getHexDigitNumber(string[i -1]) << 4;
        }

        outputValue->value.bytes[byteIndex] = value;
        byteIndex++;
    }

    return FixedPointErrorCode::NO_ERROR;
}

int compareEqualFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second) {
    return first->parts[0] == second->parts[0] && first->parts[1] == second->parts[1] &&
           first->parts[2] == second->parts[2] && first->parts[3] == second->parts[3];
}

int compareGreaterFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second) {
    if (first->parts[3] != second->parts[3]) {
        return (first->parts[3] > second->parts[3]);
    }

    if (first->parts[2] != second->parts[2]) {
        return (first->parts[2] > second->parts[2]);
    }

    if (first->parts[1] != second->parts[1]) {
        return (first->parts[1] > second->parts[1]);
    }

    return (first->parts[0] > second->parts[0]);
}

int compareLesserFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second) {
    return !compareEqualFixedPointValuesData(first, second) && !compareGreaterFixedPointValuesData(first, second);
}

int compareGreaterFixedPointValues(FixedPointValue* first, FixedPointValue* second) {
    if (first->sign != second->sign) {
        return first->sign > second->sign;
    }

    return compareGreaterFixedPointValuesData(&first->value, &second->value) ^ (first->sign == -1);
}

int compareEqualFixedPointValues(FixedPointValue* first, FixedPointValue* second) {
    if (first->sign != second->sign) {
        return 0;
    }

    return compareEqualFixedPointValuesData(&first->value, &second->value);
}

int compareGreaterEqualFixedPointValues(FixedPointValue* first, FixedPointValue* second) {
    if (first->sign != second->sign) {
        return first->sign > second->sign;
    }

    return compareEqualFixedPointValuesData(&first->value, &second->value) ||
           compareGreaterFixedPointValuesData(&first->value, &second->value) ^ (first->sign == -1);
}

FixedPointErrorCode shiftOneLeftFixedPointValuesData(FixedPointValueData* value, FixedPointValueData* output) {
    if (value->bytes[15] / 16 > 0) {
        return FixedPointErrorCode::OVERFLOW_ERROR;
    }

    for (int i = 15; i >= 0; i--) {
        int lowerCurrentValue = value->bytes[i] % 16;
        int upperPrevValue = 0;

        if (i != 0) {
            upperPrevValue = value->bytes[i - 1] / 16;
        }

        output->bytes[i] = (lowerCurrentValue << 4) + upperPrevValue;
    }

    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode shiftOneRightFixedPointValuesData(FixedPointValueData* value, FixedPointValueData* output) {
    for (int i = 0; i < 16; i++) {
        int upperCurrentValue = value->bytes[i] / 16;
        int lowerNextValue = 0;

        if (i != 15) {
            lowerNextValue = value->bytes[i + 1] % 16;
        }

        output->bytes[i] = (lowerNextValue << 4) + upperCurrentValue;
    }

    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode shiftLeftFixedPointValuesData(FixedPointValueData* value, FixedPointValueData* output, int shift) {
    assert(shift > 0);
    assert(shift < 16);

    memset(output->bytes, 0, 16 * sizeof(unsigned char));

    FixedPointErrorCode result = FixedPointErrorCode::NO_ERROR;

    if (shift & 1) {
        result = shiftOneLeftFixedPointValuesData(value, output);
    }

    if (result != FixedPointErrorCode::NO_ERROR) {
        return result;
    }

    shift -= 1;

    int indexOffset = shift / 2;

    for (int i = 15; i <= 16 - indexOffset; i--) {
        if (output->bytes[i] != 0) {
            return FixedPointErrorCode::OVERFLOW_ERROR;
        }
    }

    for (int i = 15; i >= indexOffset; i--) {
        output->bytes[i] = value->bytes[i - 1];
    }

    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode shiftRightFixedPointValuesData(FixedPointValueData* value, FixedPointValueData* output, int shift) {
    assert(shift > 0);
    assert(shift < 16);

    memset(output->bytes, 0, 16 * sizeof(unsigned char));

    FixedPointErrorCode result = FixedPointErrorCode::NO_ERROR;

    if (shift & 1) {
        result = shiftOneRightFixedPointValuesData(value, output);
    }

    if (result != FixedPointErrorCode::NO_ERROR) {
        return result;
    }

    shift -= 1;

    int indexOffset = shift / 2;

    for (int i = 0; i < 16 - indexOffset; i++) {
        output->bytes[i] = value->bytes[i + 1];
    }

    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode addFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second, FixedPointValueData* outputValue) {
    memset(outputValue->bytes, 0, 16 * sizeof(unsigned char));

    for (int i = 0; i < 4; i++) {
        unsigned long long int overflowControl = 0;
        overflowControl = first->parts[i] + second->parts[i];
        outputValue->parts[i] += overflowControl % UINT_MAX;
        if (overflowControl > UINT_MAX) {
            if (i == 3) {
                return FixedPointErrorCode::OVERFLOW_ERROR;
            }

            outputValue->parts[i + 1] += 1;
        }
    }

    return FixedPointErrorCode::NO_ERROR;
}

// FIRST SHOULD BE GREATER OR EQUAL THAN SECOND
FixedPointErrorCode substractFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second, FixedPointValueData* outputValue) {
    assert(compareGreaterFixedPointValuesData(first, second) == 1 ||
           compareEqualFixedPointValuesData(first, second) == 1);

    memset(outputValue->bytes, 0, 16 * sizeof(unsigned char));

    int negativeSign = false;

    for (int i = 0; i < 16; i++) {
        int overflowControl = 0;

        if (negativeSign) {
            overflowControl -= 1;
            negativeSign = false;
        }

        overflowControl += first->bytes[i] - second->bytes[i];

        if (overflowControl >= 0) {
            outputValue->bytes[i] += overflowControl;
        }
        else {
            negativeSign = true;
            outputValue->bytes[i] += 256 + overflowControl;
        }
    }

    assert(negativeSign == false);
    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode addFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue);
FixedPointErrorCode substractFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue);
FixedPointErrorCode multiplyFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue);
FixedPointErrorCode divideFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue);

FixedPointErrorCode addFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue) {
    memset(outputValue->value.bytes, 0, 16 * sizeof(unsigned char));

    FixedPointErrorCode result = FixedPointErrorCode::NO_ERROR;

    if (first->sign == second->sign) {
        outputValue->sign = first->sign;
        result = addFixedPointValuesData(&first->value, &second->value, &outputValue->value);
    }
    else {
        int isFirstGreaterThanSecond = compareGreaterFixedPointValuesData(&first->value, &second->value);

        if (first->sign == 1) {
            if (isFirstGreaterThanSecond) {
                outputValue->sign = 1;
                substractFixedPointValuesData(&first->value, &second->value, &outputValue->value);
            }
            else {
                outputValue->sign = -1;
                substractFixedPointValuesData(&second->value, &first->value, &outputValue->value);
            }
        }
        else {
            if (isFirstGreaterThanSecond) {
                outputValue->sign = -1;
                substractFixedPointValuesData(&first->value, &second->value, &outputValue->value);
            }
            else {
                outputValue->sign = 1;
                substractFixedPointValuesData(&second->value, &first->value, &outputValue->value);
            }
        }
    }

    return result;
}

FixedPointErrorCode substractFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue) {
    memset(outputValue->value.bytes, 0, 16 * sizeof(unsigned char));
    int isFirstGreaterThanSecond = compareGreaterFixedPointValuesData(&first->value, &second->value);

    FixedPointErrorCode result = FixedPointErrorCode::NO_ERROR;

    if (first->sign == 1) {
        if (second->sign == 1) {
            if(isFirstGreaterThanSecond) {
                outputValue->sign = 1;
                substractFixedPointValuesData(&first->value, &second->value, &outputValue->value);
            }
            else {
                outputValue->sign = -1;
                substractFixedPointValuesData(&second->value, &first->value, &outputValue->value);
            }
        }
        else {
            outputValue->sign = 1;
            result = addFixedPointValuesData(&first->value, &second->value, &outputValue->value);
        }
    }
    else {
        if (second->sign == 1) {
            outputValue->sign = -1;
            result = addFixedPointValuesData(&first->value, &second->value, &outputValue->value);
        }
        else {
            if (isFirstGreaterThanSecond) {
                outputValue->sign = -1;
                substractFixedPointValuesData(&first->value, &second->value, &outputValue->value);
            }
            else {
                outputValue->sign = 1;
                substractFixedPointValuesData(&second->value, &first->value, &outputValue->value);
            }
        }
    }

    return result;
}

FixedPointErrorCode multiplyFixedPointValuesData(FixedPointValueData* first, FixedPointValueData* second, FixedPointValueData* outputValue) {
    unsigned char multiplyBuffer[32];
    memset(multiplyBuffer, 0, 32 * sizeof(unsigned char));
    memset(outputValue->bytes, 0, 16 * sizeof(unsigned char));

    for (int secondByteIndex = 0; secondByteIndex < 16; secondByteIndex++) {
        for (int firstByteIndex = 0; firstByteIndex < 16; firstByteIndex++) {
            unsigned int result = first->bytes[firstByteIndex] * second->bytes[secondByteIndex];
            multiplyBuffer[secondByteIndex + firstByteIndex] += result % 256;
            multiplyBuffer[secondByteIndex + firstByteIndex + 1] += result / 256;
        }
    }

    for (int i = 20; i < 32; i++) {
        if (multiplyBuffer[i] > 0) {
            return FixedPointErrorCode::OVERFLOW_ERROR;
        }
    }

    for (int i = 0; i < 16; i++) {
        outputValue->bytes[i] = multiplyBuffer[i + 4];
    }

    return FixedPointErrorCode::NO_ERROR;
}

FixedPointErrorCode multiplyFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue) {
    memset(outputValue->value.bytes, 0, 16 * sizeof(unsigned char));

    outputValue->sign = first->sign == second->sign ? 1 : -1;

    return multiplyFixedPointValuesData(&first->value, &second->value, &outputValue->value);
}

// return 0..16
int findClosestFixedPointValue(FixedPointValueData* target, FixedPointValueData* value) {
    if (compareEqualFixedPointValuesData(target, value)) {
        return 0;
    }

    assert(compareGreaterFixedPointValuesData(target, value));

    FixedPointValueData shiftedValue;
    shiftOneLeftFixedPointValuesData(value, &shiftedValue);
    if (compareEqualFixedPointValuesData(target, &shiftedValue)) {
        return 16;
    }

    if (compareLesserFixedPointValuesData(&shiftedValue, target)) {
        return 16;
    }

    FixedPointValueData multipliyer;
    FixedPointValueData multipliyedValue;

    for (int i = 1; i < 16; i++) {
        memset(multipliyer.parts, 0, 4 * sizeof(unsigned int));
        memset(multipliyedValue.parts, 0, 4 * sizeof(unsigned int));

        multipliyer.parts[1] = i;

        multiplyFixedPointValuesData(value, &multipliyer, &multipliyedValue);

        if (compareEqualFixedPointValuesData(target, &multipliyedValue)) {
            return i;
        }

        if (compareGreaterFixedPointValuesData(&multipliyedValue, target)) {
            return i - 1;
        }
    }

    return 15;
}

FixedPointErrorCode divideFixedPointValues(FixedPointValue* first, FixedPointValue* second, FixedPointValue* outputValue) {
    memset(outputValue->value.bytes, 0, 16 * sizeof(unsigned char));

    if (second->value.parts[0] == 0 && second->value.parts[1] == 0 &&
        second->value.parts[2] == 0 && second->value.parts[3] == 0) {
        return FixedPointErrorCode::ZERO_DIVISION;
    }

    if (first->value.parts[0] == 0 && first->value.parts[1] == 0 &&
        first->value.parts[2] == 0 && first->value.parts[3] == 0) {
        return FixedPointErrorCode::NO_ERROR;
    }

    outputValue->sign = first->sign == second->sign ? 1 : -1;

    if (compareEqualFixedPointValuesData(&first->value, &second->value)) {
        outputValue->value.parts[1] = 1;
        return FixedPointErrorCode::NO_ERROR;
    }

    FixedPointValueData firstCopyData = first->value;
    FixedPointValueData secondCopyData = second->value;

    int firstLeftByteIndex = 31;
    int secondLeftByteIndex = 31;

    for (int i = 15; i >= 0; i--) {
        if (firstCopyData.bytes[i] == 0) {
            firstLeftByteIndex -= 2;
        }
        else {
            (firstCopyData.bytes[i] < (1 << 4)) ? firstLeftByteIndex -= 1 : firstLeftByteIndex;
            break;
        }
    }

    for (int i = 15; i >= 0; i--) {
        if (secondCopyData.bytes[i] == 0) {
            secondLeftByteIndex -= 2;
        }
        else {
            (secondCopyData.bytes[i] < (1 << 4)) ? secondLeftByteIndex -= 1 : secondLeftByteIndex;
            break;
        }
    }

    if (firstLeftByteIndex < 8 && secondLeftByteIndex < 8) {
        int maxLeftByteIndex = fmax(firstLeftByteIndex, secondLeftByteIndex);
        int shift = 8 - maxLeftByteIndex;

        FixedPointValueData shiftedValue;
        shiftLeftFixedPointValuesData(&firstCopyData, &shiftedValue, shift);
        memcpy(firstCopyData.parts, shiftedValue.parts, 4 * sizeof(unsigned int));

        shiftLeftFixedPointValuesData(&secondCopyData, &shiftedValue, shift);
        memcpy(secondCopyData.parts, shiftedValue.parts, 4 * sizeof(unsigned int));
    }

    int outputByteIndex = 8;

    FixedPointValueData modValue = firstCopyData;
    memcpy(modValue.parts, firstCopyData.parts, 4 * sizeof(unsigned int));
    FixedPointValueData divisor = secondCopyData;
    memcpy(divisor.parts, secondCopyData.parts, 4 * sizeof(unsigned int));

    while (true) {
        if (compareEqualFixedPointValuesData(&modValue, &divisor)) {
            outputValue->value.bytes[outputByteIndex / 2] += 1 * (outputByteIndex & 1 ? (16) : 1);
            return FixedPointErrorCode::NO_ERROR;
        }

        if (compareLesserFixedPointValuesData(&modValue, &divisor)) {
            outputByteIndex--;
            if (outputByteIndex < 0) {
                return FixedPointErrorCode::NO_ERROR;
            }

            FixedPointValueData temp;
            shiftOneRightFixedPointValuesData(&divisor, &temp);
            memcpy(divisor.parts, temp.parts, 4 * sizeof(unsigned int));
            continue;
        }

        int multipliyer = findClosestFixedPointValue(&modValue, &divisor);

        if (multipliyer == 16) {
            outputByteIndex++;
            if (outputByteIndex > 31) {
                return FixedPointErrorCode::OVERFLOW_ERROR;
            }

            FixedPointValueData temp;
            shiftOneLeftFixedPointValuesData(&divisor, &temp);
            memcpy(divisor.parts, temp.parts, 4 * sizeof(unsigned int));
            continue;
        }

        FixedPointValueData multipliyerFixedValue;
        memset(multipliyerFixedValue.bytes, 0, 16 * sizeof(unsigned char));
        multipliyerFixedValue.parts[1] = multipliyer;

        FixedPointValueData multipliyedDivisor;
        memset(multipliyedDivisor.bytes, 0, 16 * sizeof(unsigned char));
        multiplyFixedPointValuesData(&divisor, &multipliyerFixedValue, &multipliyedDivisor);

        outputValue->value.bytes[outputByteIndex / 2] += multipliyer * (outputByteIndex & 1 ? (16) : 1);

        FixedPointValueData result;
        memset(result.bytes, 0, 16 * sizeof(unsigned char));
        substractFixedPointValuesData(&modValue, &multipliyedDivisor, &result);
        memcpy(modValue.parts, result.parts, 4 * sizeof(unsigned int));

        if (modValue.parts[0] == 0 && modValue.parts[1] == 0 &&
            modValue.parts[2] == 0 && modValue.parts[3] == 0) {
            return FixedPointErrorCode::NO_ERROR;
        }
    }

    return FixedPointErrorCode::NO_ERROR;
}

const char* evaluateExpression(const char* expression, FixedPointErrorCode* errorCode) {
    if (expression == NULL) {
        return NULL;
    }

    int countOfOperations = 0;
    const char* iterator = expression;

    int isNextNegative = 0;
    if (expression[0] == '-') {
        isNextNegative = 1;
        iterator++;
    }

    while (*iterator != '\0') {
        if (*iterator == '+' || *iterator == '-' || *iterator == '/' || *iterator == '*') {
            countOfOperations = countOfOperations + 1;
        }

        iterator++;
    }

    if (countOfOperations == 0) {
        int length = strlen(expression);

        char* copyOfString = (char*)malloc((length + 1) * sizeof(char));
        memcpy(copyOfString, expression, length + 1);
        return copyOfString;
    }

    int countOfNumbers = countOfOperations + 1;

    FixedPointValue* fixedPointValues = (FixedPointValue*)malloc(countOfNumbers * sizeof(FixedPointValue));
    if (fixedPointValues == NULL) {
        return NULL;
    }

    char* operations = (char*)malloc(countOfOperations * sizeof(char));
    if (operations == NULL) {
        free(fixedPointValues);
        return NULL;
    }

    iterator = expression;

    if (isNextNegative) {
        iterator++;
    }

    const char* currentFixedPointValueIterator = iterator;
    int currentFloat = 0;

    FixedPointErrorCode error = FixedPointErrorCode::NO_ERROR;

    while (true) {
        if (*iterator == '\0') {
            error = parseFixedPointValue(currentFixedPointValueIterator, (iterator - currentFixedPointValueIterator), &(fixedPointValues[currentFloat]));
            if (isNextNegative) {
                fixedPointValues[currentFloat].sign = -1;
            }

            break;
        }

        if (*iterator == '+' || *iterator == '-' || *iterator == '/' || *iterator == '*') {
            error = parseFixedPointValue(currentFixedPointValueIterator, (iterator - currentFixedPointValueIterator), &(fixedPointValues[currentFloat]));
            if (isNextNegative) {
                fixedPointValues[currentFloat].sign = -1;
                isNextNegative = 0;
            }

            if (*iterator == '-') {
                isNextNegative = 1;
                operations[currentFloat] = '+';
            }
            else {
                operations[currentFloat] = *iterator;
            }

            currentFixedPointValueIterator = iterator + 1;
            currentFloat++;
        }

        if (error != FixedPointErrorCode::NO_ERROR) {
            break;
        }

        iterator++;
    }

    if (error != FixedPointErrorCode::NO_ERROR) {
        *errorCode = error;
        free(fixedPointValues);
        free(operations);
        return NULL;
    }

    for (int i = countOfOperations - 1; i >= 0; i--) {
        if (operations[i] == '*' || operations[i] == '/') {
            FixedPointValue* first = &(fixedPointValues[i]);
            FixedPointValue* second = &(fixedPointValues[i + 1]);
            FixedPointValue output;

            if (operations[i] == '*') {
                error = multiplyFixedPointValues(first, second, &output);
            }
            else {
                error = divideFixedPointValues(first, second, &output);
            }

            if (error != FixedPointErrorCode::NO_ERROR) {
                break;
            }

            fixedPointValues[i] = output;

            for (int j = i + 1; j < countOfNumbers - 1; j++) {
                fixedPointValues[j] = fixedPointValues[j + 1];
            }

            for (int j = i; j < countOfOperations - 1; j++) {
                operations[j] = operations[j + 1];
            }

            countOfNumbers--;
            countOfOperations--;
        }
    }

    if (error != FixedPointErrorCode::NO_ERROR) {
        *errorCode = error;
        free(fixedPointValues);
        free(operations);
        return NULL;
    }

    for (int i = countOfOperations - 1; i >= 0; i--) {
        FixedPointValue* first = &(fixedPointValues[i]);
        FixedPointValue* second = &(fixedPointValues[i + 1]);
        FixedPointValue output;

        assert(operations[i] == '+');

        if (operations[i] == '+') {
            error = addFixedPointValues(first, second, &output);
        }
        else {
            error = substractFixedPointValues(first, second, &output);
        }

        if (error != FixedPointErrorCode::NO_ERROR) {
            break;
        }

        fixedPointValues[i] = output;

        for (int j = i + 1; j < countOfNumbers - 1; j++) {
            fixedPointValues[j] = fixedPointValues[j + 1];
        }

        for (int j = i; j < countOfOperations - 1; j++) {
            operations[j] = operations[j + 1];
        }

        countOfNumbers--;
        countOfOperations--;
    }

    if (error != FixedPointErrorCode::NO_ERROR) {
        *errorCode = error;
        free(fixedPointValues);
        free(operations);
        return NULL;
    }

    FixedPointValue& resultValue = fixedPointValues[0];

    int leftIndex = 4;
    int rightIndex = 4;

    for (int i = 0; i < 4; i++) {
        if (resultValue.value.bytes[i] != 0) {
            leftIndex = i;
            break;
        }
    }

    int hasPoint = leftIndex != 4;
    int hasSign =resultValue.sign == -1;

    for (int i = 15; i >= 4; i--) {
        if (resultValue.value.bytes[i] != 0) {
            rightIndex = i;
            break;
        }
    }

    int size = (rightIndex - leftIndex + 1) * 2 + 1; // byte for '\0'
    size += hasPoint;
    size += hasSign;

    char* resultString = (char*)malloc(size * sizeof(char));

    int symbolIndex = size - 2;
    for (int i = leftIndex; i < 4; i++) {
        int lowerValue = resultValue.value.bytes[i] % 16;
        int upperValue = resultValue.value.bytes[i] / 16;

        resultString[symbolIndex] = (i == leftIndex && lowerValue == 0) ? '\0' : getHexDigitSymbol(lowerValue);
        resultString[symbolIndex - 1] = getHexDigitSymbol(upperValue);

        symbolIndex -= 2;
    }

    if (hasPoint) {
        resultString[symbolIndex] = '.';
        symbolIndex--;
    }

    for (int i = 4; i <= rightIndex; i++) {
        int lowerValue = resultValue.value.bytes[i] % 16;
        int upperValue = resultValue.value.bytes[i] / 16;

        resultString[symbolIndex] = getHexDigitSymbol(lowerValue);

        if (!(upperValue == 0 && i == rightIndex)) {
            resultString[symbolIndex - 1] = getHexDigitSymbol(upperValue);
        }

        symbolIndex -= 2;
    }

    if (hasSign) {
        resultString[0] = '-';
    }

    resultString[size - 1] = '\0';

    free(fixedPointValues);
    free(operations);

    return resultString;
}
