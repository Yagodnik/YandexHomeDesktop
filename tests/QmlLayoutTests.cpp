#include "QmlTestEnvironment.h"
#include <QtQuickTest/quicktest.h>

class QmlLayoutTestSetup : public QObject {
  Q_OBJECT

public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    initializeQmlTestEnvironment(engine);
  }
};

QUICK_TEST_MAIN_WITH_SETUP(QmlLayoutTests, QmlLayoutTestSetup)

#include "QmlLayoutTests.moc"
