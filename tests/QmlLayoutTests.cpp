#include "QmlTestEnvironment.h"
#include <QtQuickTest/quicktest.h>
#include <QQmlContext>

class QmlLayoutTestSetup : public QObject {
  Q_OBJECT

public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    initializeQmlTestEnvironment(engine);
    engine->rootContext()->setContextProperty("settingsPreviewPath",
      QCoreApplication::applicationDirPath() + "/settings-preview");
  }
};

QUICK_TEST_MAIN_WITH_SETUP(QmlLayoutTests, QmlLayoutTestSetup)

#include "QmlLayoutTests.moc"
