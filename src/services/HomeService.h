#pragma once

#include "yh/appservices_export.h"

#include <QObject>
#include <QUuid>
#include <optional>
#include "api/IHomeApi.h"

class APPSERVICES_EXPORT HomeService final : public QObject {
  Q_OBJECT
public:
  enum class LoadState { NotLoaded, Loading, Ready, Error };
  explicit HomeService(IHomeApi* api, QObject* parent = nullptr);

  [[nodiscard]] LoadState GetLoadState() const;
  [[nodiscard]] const UserInfo& GetSnapshot() const;
  [[nodiscard]] QString GetCurrentHousehold() const;
  [[nodiscard]] QString GetCurrentHouseholdName() const;
  void EnsureLoaded();
  void Refresh();
  void SelectHousehold(const QString& id);
  void Reset();
  // One-shot consumers own delivery without changing the GUI snapshot/selection.
  void ReadHome(QObject* context, ApiResultHandler<UserInfo> handler);

signals:
  void loadStateChanged();
  void snapshotChanged();
  void currentHouseholdChanged();

private:
  void SetLoadState(LoadState state);
  IHomeApi* api_;
  LoadState state_ = LoadState::NotLoaded;
  UserInfo snapshot_{};
  QString current_household_;
  std::optional<QUuid> request_;
};
