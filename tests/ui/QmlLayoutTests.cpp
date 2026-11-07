#include "QmlTestEnvironment.h"
#include <QtQuickTest/quicktest.h>
#include <QQmlContext>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QWindow>
#include <QTemporaryDir>
#include <QImage>
#include <memory>

class QmlLayoutTestSetup : public QObject {
  Q_OBJECT

public:
  Q_INVOKABLE QVariantMap snapshotInfo(const QUrl& url) const {
    const QImage image(url.toLocalFile());
    bool varied = false;
    for (int y = 0; y < image.height() && !varied; y += 8)
      for (int x = 0; x < image.width(); x += 8)
        if (image.pixel(x, y) != image.pixel(0, 0)) {
          varied = true;
          break;
        }
    return {{"width", image.width()}, {"height", image.height()}, {"varied", varied}};
  }

  Q_INVOKABLE int cursorShape(QObject* object) const {
    const auto* window = qobject_cast<QWindow*>(object);
    return window ? window->cursor().shape() : -1;
  }

public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    initializeQmlTestEnvironment(engine);
    engine->rootContext()->setContextProperty("uiTestHelper", this);
    const auto video_directory = std::make_shared<QTemporaryDir>();
    connect(engine, &QObject::destroyed, [video_directory] {});
    for (const auto* name : {"camera.m3u8", "camera.ts"}) {
      if (!QFile::copy(QString(":/debug/") + name, video_directory->filePath(name)))
        qFatal("Cannot prepare local HLS test clip");
    }
    engine->rootContext()->setContextProperty(
        "demoVideoSource", QUrl::fromLocalFile(video_directory->filePath("camera.m3u8")));
    engine->rootContext()->setContextProperty(
        "cameraSnapshotTestFile", QUrl::fromLocalFile(video_directory->filePath("snapshot.png")));
    engine->rootContext()->setContextProperty(
        "settingsPreviewPath", QCoreApplication::applicationDirPath() + "/settings-preview");
    QFile fixture(":/debug/api.json");
    if (!fixture.open(QIODevice::ReadOnly))
      qFatal("Cannot load bundled demo fixture");
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
