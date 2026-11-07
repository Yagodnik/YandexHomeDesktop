#pragma once

#include <QJsonObject>
#include <QHash>
#include <QTemporaryDir>
#include <memory>
#include "api/IHomeApi.h"
#include "api/IAccountApi.h"

// Local fixture playback. No transport, token provider, or credential storage.
class FixtureApi final : public QObject, public IHomeApi, public IAccountApi {
public:
  explicit FixtureApi(QString path, QObject* parent = nullptr);
  [[nodiscard]] ApiResult<void> Validate() const;
  void GetUserInfo(QObject* context, ApiResultHandler<UserInfo> handler) override;
  void GetScenarios(QObject* context, ApiResultHandler<QList<ScenarioObject>> handler) override;
  void GetDeviceInfo(const QString& id, QObject* context, ApiResultHandler<DeviceInfo> handler) override;
  void ExecuteScenario(const QString& id, QObject* context, ApiResultHandler<void> handler) override;
  void PerformActions(const QList<DeviceActionsObject>& actions, QObject* context, ApiResultHandler<void> handler) override;
  void GetVideoStream(const QString& device_id, QObject* context,
                      ApiResultHandler<VideoStream> handler) override;
  void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) override;

private:
  [[nodiscard]] ApiResult<QJsonObject> ReadFixture() const;
  [[nodiscard]] static ApiResult<UserInfo> DecodeHome(const QJsonObject& fixture);
  void ApplyStates(const QString& id, QList<CapabilityObject>& capabilities) const;
  QString path_;
  mutable QJsonObject snapshot_;
  mutable QHash<QString, QHash<int, QVariantMap>> states_;
  std::unique_ptr<QTemporaryDir> video_directory_;
};
