#pragma once

#include <QQmlEngine>
#include <QStandardItemModel>
#include <QPointer>
#include <QTest>
#include <QLocalServer>
#include <optional>
#include "models/RestViewModel.h"
#include "api/IHomeApi.h"

// Real device view-model wiring with responses controlled by QML tests.
class QmlTestDeviceApi final : public QObject, public IHomeApi {
  Q_OBJECT
  Q_PROPERTY(int requestCount MEMBER request_count_ NOTIFY requested)
  Q_PROPERTY(QString deviceId MEMBER device_id_ NOTIFY requested)
  Q_PROPERTY(QString deviceName MEMBER device_name_)
  Q_PROPERTY(bool withCapability MEMBER with_capability_)
  Q_PROPERTY(int actionCount MEMBER action_count_ NOTIFY actionSent)
  Q_PROPERTY(QString actionDeviceId MEMBER action_device_id_ NOTIFY actionSent)
  Q_PROPERTY(QVariantMap actionState MEMBER action_state_ NOTIFY actionSent)
  Q_PROPERTY(int streamRequestCount MEMBER stream_request_count_ NOTIFY streamRequested)
  Q_PROPERTY(QString streamDeviceId MEMBER stream_device_id_ NOTIFY streamRequested)
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
  void GetVideoStream(const QString& id, QObject* context,
                      ApiResultHandler<VideoStream> handler) override {
    stream_device_id_ = id;
    stream_context_ = context;
    stream_handler_ = std::move(handler);
    ++stream_request_count_;
    emit streamRequested();
  }
  Q_INVOKABLE void ReplyVideoStream(const QString& url, bool success) {
    if (!stream_context_ || !stream_handler_)
      qFatal("No pending video stream request");
    auto handler = std::move(stream_handler_);
    if (success)
      handler(VideoStream{.stream_url = url, .protocol = "hls"});
    else
      handler(std::unexpected(ApiError{ApiErrorKind::Timeout, "TIMEOUT"}));
  }

  Q_INVOKABLE void SetCapability(const QString& kind, const QVariantMap& state, const QVariantMap& parameters) {
    CapabilityObject capability{};
    if (kind == "range") capability.type = CapabilityType::Range;
    else if (kind == "color") capability.type = CapabilityType::ColorSetting;
    else if (kind == "mode") capability.type = CapabilityType::Mode;
    else if (kind == "video")
      capability.type = CapabilityType::VideoStream;
    else qFatal("Unexpected test capability kind");
    capability.retrievable = true;
    capability.state = state;
    capability.parameters = parameters;
    capability_override_ = capability;
  }
  Q_INVOKABLE void ReplyFromFixture(const QVariantMap& device);
  Q_INVOKABLE void SelectTestAccount(const QString& accountId) { emit testAccountSelected(accountId); }

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
    info.name = device_name_;
    info.status = Status::Ok;
    info.state = DeviceState::Online;
    if (with_capability_) {
      CapabilityObject capability{};
      capability.type = CapabilityType::OnOff;
      capability.state = {{"instance", "on"}, {"value", false}};
      capability.parameters = {{"instance", "on"}};
      info.capabilities = {capability};
    }
    if (capability_override_) info.capabilities.append(*capability_override_);
    handler(info);
  }
  Q_INVOKABLE void reset() {
    stream_request_count_ = 0;
    stream_device_id_.clear();
    stream_handler_ = {};
    stream_context_.clear();
    handler_ = {};
    context_.clear();
    request_count_ = 0;
    device_id_.clear();
    device_name_ = "Test device";
    with_capability_ = false;
    capability_override_.reset();
    action_count_ = 0;
    action_device_id_.clear();
    action_state_.clear();
    emit requested();
    emit actionSent();
    emit fixtureReset();
    emit streamRequested();
  }
signals:
  void fixtureReset();
  void testAccountSelected(const QString& accountId);
  void requested();
  void actionSent();
  void streamRequested();

private:
  int stream_request_count_ = 0;
  QString stream_device_id_;
  QPointer<QObject> stream_context_;
  ApiResultHandler<VideoStream> stream_handler_;
  int request_count_ = 0;
  QString device_id_;
  QString device_name_ = "Test device";
  bool with_capability_ = false;
  std::optional<CapabilityObject> capability_override_;
  int action_count_ = 0;
  QString action_device_id_;
  QVariantMap action_state_;
  QPointer<QObject> context_;
  ApiResultHandler<DeviceInfo> handler_;
};

// Scenario page interactions exercise the real service and view model without I/O.
class QmlTestScenarioApi final : public QObject, public IHomeApi {
  Q_OBJECT
  Q_PROPERTY(int executionCount MEMBER execution_count_ NOTIFY executed)
  Q_PROPERTY(QString lastScenario MEMBER last_scenario_ NOTIFY executed)
public:
  using QObject::QObject;
  Q_INVOKABLE void SetRows(const QVariantList& rows) {
    scenarios_.clear();
    execution_count_ = 0;
    last_scenario_.clear();
    for (const auto& row : rows) {
      const auto value = row.toMap();
      scenarios_.append(ScenarioObject{.id = value["id"].toString(), .name = value["name"].toString(), .is_active = value["is_active"].toBool()});
    }
    emit executed();
  }
  void GetScenarios(QObject*, ApiResultHandler<QList<ScenarioObject>> handler) override { handler(scenarios_); }
  void ExecuteScenario(const QString& id, QObject*, ApiResultHandler<void> handler) override {
    ++execution_count_;
    last_scenario_ = id;
    emit executed();
    handler(ApiResult<void>{});
  }
  void GetUserInfo(QObject*, ApiResultHandler<UserInfo>) override { qFatal("Unexpected user request"); }
  void GetDeviceInfo(const QString&, QObject*, ApiResultHandler<DeviceInfo>) override { qFatal("Unexpected device request"); }
  void PerformActions(const QList<DeviceActionsObject>&, QObject*, ApiResultHandler<void>) override { qFatal("Unexpected device action"); }
signals:
  void executed();
private:
  QList<ScenarioObject> scenarios_;
  int execution_count_ = 0;
  QString last_scenario_;
};

// Other page tests use empty local models and record calls without any I/O.
class QmlTestModel : public QStandardItemModel {
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY dataLoaded)
  Q_PROPERTY(QString name MEMBER name CONSTANT)
  Q_PROPERTY(bool isOnline MEMBER isOnline CONSTANT)
  Q_PROPERTY(QString currentHousehold MEMBER currentHousehold NOTIFY currentHouseholdChanged)
  Q_PROPERTY(QString currentHouseholdName MEMBER currentHouseholdName CONSTANT)
  Q_PROPERTY(bool notchSupported MEMBER notchSupported NOTIFY notchSupportedChanged)
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
  bool notchSupported = false;
  int currentTheme = 0;
  int callCount = 0;
  Q_PROPERTY(bool authorizationPending MEMBER authorizationPending NOTIFY called)
  Q_PROPERTY(QString authorizationCode MEMBER authorizationCode NOTIFY called)
  Q_PROPERTY(QString authorizationUrl MEMBER authorizationUrl NOTIFY called)
  bool authorizationPending = false;
  QString authorizationCode;
  QString authorizationUrl;
  QString lastCall;
  QString lastRoute;

  Q_INVOKABLE void RequestData() { record("RequestData"); }
  Q_INVOKABLE void LoadData() { record("LoadData"); }
  Q_INVOKABLE void CancelAuthorization() { record("CancelAuthorization"); }
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
  void notchSupportedChanged();
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

class QmlTestRestServer final : public QObject {
  Q_OBJECT
  Q_PROPERTY(int disableCount MEMBER disable_count_ NOTIFY changed)
public:
  explicit QmlTestRestServer(QObject* parent = nullptr);
  Q_INVOKABLE void SetRunning(bool running) { running_ = running; }
  QString ControlName() const { return server_.serverName(); }
signals:
  void changed();
private:
  QLocalServer server_;
  bool running_ = false;
  int disable_count_ = 0;
};
