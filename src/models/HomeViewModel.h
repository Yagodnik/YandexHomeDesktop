#pragma once

#include "DevicesModel/DevicesModel.h"
#include "RoomsModel/RoomsModel.h"
#include "RoomsModel/RoomsFilterModel.h"
#include "HouseholdsModel/HouseholdsModel.h"
#include "services/HomeService.h"

class HomeViewModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(PageState state READ GetState NOTIFY stateChanged)
  Q_PROPERTY(bool loading READ IsLoading NOTIFY stateChanged)
  Q_PROPERTY(DevicesModel* devices READ GetDevices CONSTANT)
  Q_PROPERTY(RoomsFilterModel* rooms READ GetRooms CONSTANT)
  Q_PROPERTY(HouseholdsModel* households READ GetHouseholds CONSTANT)
  Q_PROPERTY(QString currentHousehold READ GetCurrentHousehold WRITE SelectHousehold NOTIFY currentHouseholdChanged)
  Q_PROPERTY(QString currentHouseholdName READ GetCurrentHouseholdName NOTIFY currentHouseholdChanged)
public:
  enum PageState { Loading, Error, Ready };
  Q_ENUM(PageState)
  explicit HomeViewModel(HomeService* service, QObject* parent = nullptr);

  [[nodiscard]] PageState GetState() const;
  [[nodiscard]] bool IsLoading() const;
  [[nodiscard]] DevicesModel* GetDevices();
  [[nodiscard]] RoomsFilterModel* GetRooms();
  [[nodiscard]] HouseholdsModel* GetHouseholds();
  [[nodiscard]] QString GetCurrentHousehold() const;
  [[nodiscard]] QString GetCurrentHouseholdName() const;
  Q_INVOKABLE void EnsureLoaded();
  Q_INVOKABLE void Refresh();
  Q_INVOKABLE void SelectHousehold(const QString& id);

signals:
  void stateChanged();
  void currentHouseholdChanged();

private:
  void UpdateSnapshot();
  void UpdateSelection();
  HomeService* service_;
  DevicesModel devices_;
  RoomsModel rooms_;
  HouseholdsModel households_;
  RoomsFilterModel filtered_rooms_;
};
