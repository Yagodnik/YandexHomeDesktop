#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>
#include <QDebug>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  if (argc != 3) {
    qCritical() << "Expected a QML page and build import directory";
    return 1;
  }

  QQmlEngine engine;
  engine.addImportPath(QString::fromLocal8Bit(argv[2]));
  QQmlComponent page(&engine, QUrl::fromLocalFile(QString::fromLocal8Bit(argv[1])),
                     QQmlComponent::PreferSynchronous);
  if (page.status() != QQmlComponent::Ready) {
    qCritical().noquote() << page.errorString();
    return 1;
  }
  return 0;
}
