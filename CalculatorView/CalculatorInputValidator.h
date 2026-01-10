#ifndef CALCULATORINPUTVALIDATOR_H
#define CALCULATORINPUTVALIDATOR_H

#include <QValidator>
#include <QQmlEngine>

class CalculatorInputValidator : public QValidator
{
    Q_OBJECT
public:
    CalculatorInputValidator();

    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString& string) const override;

    Q_INVOKABLE QString appendSymbol(const QString& input, QChar symbol) const;
    Q_INVOKABLE QString removeSymbol(const QString& input) const;
    Q_INVOKABLE QString inverseFirst(const QString& input) const;
};

#endif // CALCULATORINPUTVALIDATOR_H
