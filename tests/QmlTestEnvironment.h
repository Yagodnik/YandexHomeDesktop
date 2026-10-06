#pragma once

#include <QQmlEngine>
#include <QStandardItemModel>
#include <QPointer>
#include <QTest>
#include "api/IHomeApi.h"

// Real device view-model wiring with responses controlled by QML tests.
class QmlTestDeviceApi final : public QObject, public IHomeApi {
  Q_OBJECT
  Q_PROPERTY(int requestCount MEMBER request_count_ NOTIFY requested)
  Q_PROPERTY(QString deviceId MEMBER device_id_ NOTIFY requested)
  Q_PROPERTY(bool withCapability MEMBER with_capability_)
  Q_PROPERTY(int actionCount MEMBER action_count_ NOTIFY actionSent)
  Q_PROPERTY(QString actionDeviceId MEMBER action_device_id_ NOTIFY actionSent)
  Q_PROPERTY(QVariantMap actionState MEMBER action_state_ NOTIFY actionSent)
public:
  using QObject::QObject;
  void GetDeviceInfo(const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) override {
    device_id_ = id;
    context_ = context;
    handler_ = std::move(handler);
    ++request_count_;
    emit requested();
  }
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject*, ApiResultHandler<void> handler) override {
    if (actions.size() != 1 || actions[0].actions.size() != 1) {
      qFatal("Unexpected action payload");
    }
    action_device_id_ = actions[0].id;
    action_state_ = actions[0].actions[0].state;
    ++action_count_;
    emit actionSent();
    handler(ApiResult<void>{});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override { qFatal("Unexpected user request"); }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>>) override { qFatal("Unexpected scenarios request"); }
  void ExecuteScenario(const QString&, QObject*, ApiResultHandler<void>) override { qFatal("Unexpected scenario action"); }

  Q_INVOKABLE void ReplyDevice(bool success) {
    if (!context_ || !handler_) {
      qFatal("No pending device request");
    }
    auto handler = std::move(handler_);
    if (!success) {
      // Expected application diagnostics; QML warnings still fail the test.
      QTest::ignoreMessage(QtWarningMsg, "PropertiesModel: Failed to update properties. Error: \"TEST_ERROR\"");
      QTest::ignoreMessage(QtCriticalMsg, "DeviceDataModel: Cant receive device info due to:");
      QTest::ignoreMessage(QtCriticalMsg, "\"TEST_ERROR\"");
      handler(std::unexpected(ApiError{ApiErrorKind::Timeout, "TEST_ERROR"}));
      return;
    }
    DeviceInfo info{};
    info.id = device_id_;
    info.name = "Test device";
    info.status = Status::Ok;
    info.state = DeviceState::Online;
    if (with_capability_) {
      CapabilityObject capability{};
      capability.type = CapabilityType::OnOff;
      capability.state = {{"instance", "on"}, {"value", false}};
      capability.parameters = {{"instance", "on"}};
      info.capabilities = {capability};
    }
    handler(info);
  }
  Q_INVOKABLE void reset() {
    handler_ = {};
    context_.clear();
    request_count_ = 0;
    device_id_.clear();
    with_capability_ = false;
    action_count_ = 0;
    action_device_id_.clear();
    action_state_.clear();
    emit requested();
    emit actionSent();
  }
signals:
  void requested();
  void actionSent();
private:
  int request_count_ = 0;
  QString device_id_;
  bool with_capability_ = false;
  int action_count_ = 0;
  QString action_device_id_;
  QVariantMap action_state_;
  QPointer<QObject> context_;
  ApiResultHandler<DeviceInfo> handler_;
};

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
