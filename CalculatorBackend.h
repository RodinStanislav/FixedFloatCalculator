#ifndef CALCULATORBACKEND_H
#define CALCULATORBACKEND_H

#include "CalculatorView/ICalculatorBackend.h"

class CalculatorBackend : public ICalculatorBackend
{
    Q_OBJECT
public:
    explicit CalculatorBackend(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE QString evaluate(const QString& expression) override;
};

#endif // CALCULATORBACKEND_H
