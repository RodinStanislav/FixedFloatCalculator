#ifndef ICALCULATORBACKEND_H
#define ICALCULATORBACKEND_H

#include <QObject>

class ICalculatorBackend : public QObject
{
    Q_OBJECT
public:
    explicit ICalculatorBackend(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE virtual QString evaluate(const QString& expression) = 0;

signals:
    void errorRaised(const QString& error);
};

#endif // CALCULATORBACKEND_H
