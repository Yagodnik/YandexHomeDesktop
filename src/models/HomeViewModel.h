#pragma once

#include "DevicesModel/DevicesModel.h"
#include "RoomsModel/RoomsModel.h"
#include "RoomsModel/RoomsFilterModel.h"
#include "HouseholdsModel/HouseholdsModel.h"
#include "services/HomeService.h"

class Settings;
class AccountModel;

class HomeViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(PageState state READ GetState NOTIFY stateChanged)
  Q_PROPERTY(bool loading READ IsLoading NOTIFY stateChanged)
  Q_PROPERTY(DevicesModel* devices READ GetDevices CONSTANT)
  Q_PROPERTY(DevicesModel* favorites READ GetFavorites CONSTANT)
  Q_PROPERTY(bool favoritesCollapsed READ AreFavoritesCollapsed WRITE SetFavoritesCollapsed NOTIFY favoritesCollapsedChanged)
  Q_PROPERTY(bool preferencesAvailable READ ArePreferencesAvailable NOTIFY preferencesAvailableChanged)
  Q_PROPERTY(RoomsFilterModel* rooms READ GetRooms CONSTANT)
  Q_PROPERTY(HouseholdsModel* households READ GetHouseholds CONSTANT)
  Q_PROPERTY(QString currentHousehold READ GetCurrentHousehold WRITE SelectHousehold NOTIFY currentHouseholdChanged)
  Q_PROPERTY(QString currentHouseholdName READ GetCurrentHouseholdName NOTIFY currentHouseholdChanged)
public:
  enum PageState { Loading, Error, Ready };
  Q_ENUM(PageState)
  explicit HomeViewModel(HomeService* service, QObject* parent = nullptr);
  HomeViewModel(HomeService* service, Settings* settings, AccountModel* account, QObject* parent = nullptr);

  [[nodiscard]] PageState GetState() const;
  [[nodiscard]] bool IsLoading() const;
  [[nodiscard]] DevicesModel* GetDevices();
  [[nodiscard]] DevicesModel* GetFavorites();
  [[nodiscard]] bool AreFavoritesCollapsed() const;
  void SetFavoritesCollapsed(bool collapsed);
  [[nodiscard]] bool ArePreferencesAvailable() const;
  [[nodiscard]] RoomsFilterModel* GetRooms();
  [[nodiscard]] HouseholdsModel* GetHouseholds();
  [[nodiscard]] QString GetCurrentHousehold() const;
  [[nodiscard]] QString GetCurrentHouseholdName() const;
  Q_INVOKABLE void EnsureLoaded();
  Q_INVOKABLE void Refresh();
  Q_INVOKABLE void SelectHousehold(const QString& id);
  Q_INVOKABLE void SetDeviceFavorite(const QString& id, bool favorite);
  Q_INVOKABLE void SetRoomCollapsed(const QString& id, bool collapsed);

signals:
  void stateChanged();
  void currentHouseholdChanged();
  void preferencesAvailableChanged();
  void favoritesCollapsedChanged();

private:
  void UpdateSnapshot();
  void UpdateSelection();
  void UpdateAccount();
  void UpdateFavorites();
  HomeService* service_;
  Settings* settings_;
  AccountModel* account_;
  QString account_id_;
  QStringList favorite_ids_;
  QStringList collapsed_room_ids_;
  bool favorites_collapsed_ = false;
  DevicesModel devices_;
  DevicesModel favorites_;
  RoomsModel rooms_;
  HouseholdsModel households_;
  RoomsFilterModel filtered_rooms_;
};
