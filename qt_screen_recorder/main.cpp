#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "src/backend.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  QQuickStyle::setStyle("Material");

  qmlRegisterType<BackEnd>("Backend", 1, 0, "BackEnd");

  QQmlApplicationEngine engine;
  const QUrl url(u"qrc:/main.qml"_qs);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}
