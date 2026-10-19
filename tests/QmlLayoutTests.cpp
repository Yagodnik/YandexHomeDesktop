#include "QmlTestEnvironment.h"
#include <QtQuickTest/quicktest.h>
#include <QQmlContext>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QWindow>

class QmlLayoutTestSetup : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE int cursorShape(QObject* object) const {
    const auto* window = qobject_cast<QWindow*>(object);
    return window ? window->cursor().shape() : -1;
  }

public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    initializeQmlTestEnvironment(engine);
    engine->rootContext()->setContextProperty("uiTestHelper", this);
    engine->rootContext()->setContextProperty("settingsPreviewPath",
      QCoreApplication::applicationDirPath() + "/settings-preview");
    QFile fixture(":/debug/api.json");
    if (!fixture.open(QIODevice::ReadOnly)) qFatal("Cannot load bundled demo fixture");
    const auto data = QJsonDocument::fromJson(fixture.readAll()).object();
    const auto details = data["device_info"].toObject();
    QVariantList devices;
    for (const auto& summary : data["user_info"].toObject()["devices"].toArray()) {
      devices.append(details[summary.toObject()["id"].toString()].toObject().toVariantMap());
    }
    engine->rootContext()->setContextProperty("demoDevices", devices);
  }
};

QUICK_TEST_MAIN_WITH_SETUP(QmlLayoutTests, QmlLayoutTestSetup)

#include "QmlLayoutTests.moc"
