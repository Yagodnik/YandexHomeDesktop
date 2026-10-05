#pragma once

#include <QQmlEngine>
#include <QStandardItemModel>

// Page tests use empty local models and record service calls without any I/O.
class QmlTestModel : public QStandardItemModel {
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY dataLoaded)
  Q_PROPERTY(QString name MEMBER name CONSTANT)
  Q_PROPERTY(bool isOnline MEMBER isOnline CONSTANT)
  Q_PROPERTY(QString currentHousehold MEMBER currentHousehold NOTIFY currentHouseholdChanged)
  Q_PROPERTY(QString currentHouseholdName MEMBER currentHouseholdName CONSTANT)
  Q_PROPERTY(bool trayModeEnabled MEMBER trayModeEnabled NOTIFY trayModeEnabledChanged)
  Q_PROPERTY(int currentTheme MEMBER currentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(int callCount MEMBER callCount NOTIFY called)
  Q_PROPERTY(QString lastCall MEMBER lastCall NOTIFY called)
  Q_PROPERTY(QString lastRoute MEMBER lastRoute NOTIFY called)

public:
  using QStandardItemModel::QStandardItemModel;
  QString name = "Test device";
  bool isOnline = true;
  QString currentHousehold = "test-household";
  QString currentHouseholdName = "Test home";
  bool trayModeEnabled = false;
  int currentTheme = 0;
  int callCount = 0;
  QString lastCall;
  QString lastRoute;

  Q_INVOKABLE void RequestData() { record("RequestData"); }
  Q_INVOKABLE void LoadData() { record("LoadData"); }
  Q_INVOKABLE void AttemptAuthorization() { record("AttemptAuthorization"); }
  Q_INVOKABLE void AttemptLocalAuthorization() { record("AttemptLocalAuthorization"); }
  Q_INVOKABLE void Logout() { record("Logout"); }
  Q_INVOKABLE void TryReloadDevice() { record("TryReloadDevice"); }
  Q_INVOKABLE void ForgetDevice() { record("ForgetDevice"); }
  Q_INVOKABLE void ShowOnlyInTray() { record("ShowOnlyInTray"); }
  Q_INVOKABLE void ShowAsApp() { record("ShowAsApp"); }
  Q_INVOKABLE void ExecuteScenario(int) { record("ExecuteScenario"); }
  Q_INVOKABLE void navigateTo(const QString& route) {
    lastRoute = route;
    record("navigateTo");
  }
  Q_INVOKABLE void goBack() { record("goBack"); }
  Q_INVOKABLE QString GetLastErrorCode() const { return "TEST_ERROR"; }
  Q_INVOKABLE QString GetName() const { return "Test user"; }
  Q_INVOKABLE QString GetEmail() const { return "test@example.invalid"; }
  Q_INVOKABLE QString GetAvatarUrl() const { return {}; }
  Q_INVOKABLE QVariant GetDeviceError(const QString&) const { return {}; }
  Q_INVOKABLE void resetCalls() {
    callCount = 0;
    lastCall.clear();
    lastRoute.clear();
    emit called();
  }

signals:
  void dataLoaded();
  void dataLoadingFailed();
  void initialized();
  void initializeFailed();
  void errorOccurred(const QString& code);
  void scenarioExecutionFailed();
  void authorized();
  void currentHouseholdChanged();
  void trayModeEnabledChanged();
  void currentThemeChanged();
  void called();

private:
  void record(const QString& call) {
    lastCall = call;
    ++callCount;
    emit called();
  }
};

void initializeQmlTestEnvironment(QQmlEngine* engine);
