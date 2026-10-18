#include <QGuiApplication>
#include <QProcess>
#include <QTest>
#include "app/shortcuts/QHotkeyRegistrar.h"

namespace {
int Combination() {
  return QKeyCombination(Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier, Qt::Key_F19)
      .toCombined();
}
int Probe() {
  QProcess process;
  process.start(QCoreApplication::applicationFilePath(), {"--probe"});
  if (!process.waitForFinished(5'000))
    return -1;
  return process.exitCode();
}
} // namespace

class NativeShortcutTests final : public QObject {
  Q_OBJECT
private slots:
  void NativeOwnershipConflictAndReleaseAcrossProcesses() {
    QHotkeyRegistrar registrar;
    if (!registrar.Register(Combination(), [] {}))
      QSKIP("Native test combination is unavailable on this desktop");
    QCOMPARE(Probe(), 1);
    QVERIFY(registrar.Clear());
    QCOMPARE(Probe(), 0);
  }
  void AdapterDeliversActivationAndClearsRegistrations() {
    QHotkeyRegistrar registrar;
    int calls = 0;
    if (!registrar.Register(Combination(), [&calls] { ++calls; }))
      QSKIP("Native test combination is unavailable on this desktop");
    auto* hotkey = registrar.findChild<QHotkey*>();
    QVERIFY(hotkey);
    QVERIFY(QMetaObject::invokeMethod(hotkey, "activated", Qt::DirectConnection));
    QCOMPARE(calls, 1);
    QVERIFY(QMetaObject::invokeMethod(hotkey, "released", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(hotkey, "activated", Qt::DirectConnection));
    QCOMPARE(calls, 2);
    QVERIFY(registrar.Clear());
    QVERIFY(registrar.findChildren<QHotkey*>().isEmpty());
  }
};
int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  if (app.arguments().contains("--probe")) {
    QHotkeyRegistrar registrar;
    return registrar.Register(Combination(), [] {}) ? 0 : 1;
  }
  NativeShortcutTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "NativeShortcutTests.moc"
