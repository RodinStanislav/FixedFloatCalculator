#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QFile>
#include <QFont>
#include <QFontDatabase>

#include "CalculatorBackend.h"
#include "CalculatorView/CalculatorInputValidator.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  if (app.font().defaultFamily().isEmpty()) {
    QFile fontFile(":/Resources/Fonts/DejaVuSans.ttf");
    assert(fontFile.exists());
    assert(fontFile.open(QIODevice::ReadOnly));
    QByteArray fileContent = fontFile.readAll();
    fontFile.close();

    QFontDatabase::addApplicationFontFromData(fileContent);

    auto fontFamilies = QFontDatabase::families();
    assert(!fontFamilies.isEmpty());

    QFont defaultFont("DejaVuSans", 12);
    app.setFont(defaultFont);
  }

  CalculatorBackend calculatorBackend;

  // the QML_ELEMENT way is currently not working
  // (https://bugreports.qt.io/browse/QTBUG-93443)
  qmlRegisterType<CalculatorInputValidator>("CalculatorInputValidator", 1, 0,
                                            "CalculatorInputValidator");

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.rootContext()->setContextProperty("calculatorBackend",
                                           &calculatorBackend);

  engine.loadFromModule("Calculator", "Main");

  return app.exec();
}
