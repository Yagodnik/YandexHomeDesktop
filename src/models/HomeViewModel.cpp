#include "HomeViewModel.h"

HomeViewModel::HomeViewModel(HomeService* service, QObject* parent)
  : QObject(parent), service_(service), devices_(this), rooms_(this), households_(this), filtered_rooms_(this) {
  filtered_rooms_.setSourceModel(&rooms_);
  connect(service_, &HomeService::loadStateChanged, this, &HomeViewModel::stateChanged);
  connect(service_, &HomeService::snapshotChanged, this, &HomeViewModel::UpdateSnapshot);
  connect(service_, &HomeService::currentHouseholdChanged, this, &HomeViewModel::UpdateSelection);
  UpdateSnapshot();
  UpdateSelection();
}

HomeViewModel::PageState HomeViewModel::GetState() const {
  switch (service_->GetLoadState()) {
    case HomeService::LoadState::NotLoaded:
    case HomeService::LoadState::Loading: return Loading;
    case HomeService::LoadState::Error: return Error;
    case HomeService::LoadState::Ready: return Ready;
  }
  return Error;
}

bool HomeViewModel::IsLoading() const { return GetState() == Loading; }
DevicesModel* HomeViewModel::GetDevices() { return &devices_; }
RoomsFilterModel* HomeViewModel::GetRooms() { return &filtered_rooms_; }
HouseholdsModel* HomeViewModel::GetHouseholds() { return &households_; }
QString HomeViewModel::GetCurrentHousehold() const { return service_->GetCurrentHousehold(); }
QString HomeViewModel::GetCurrentHouseholdName() const { return service_->GetCurrentHouseholdName(); }
void HomeViewModel::EnsureLoaded() { service_->EnsureLoaded(); }
void HomeViewModel::Refresh() { service_->Refresh(); }
void HomeViewModel::SelectHousehold(const QString& id) { service_->SelectHousehold(id); }

void HomeViewModel::UpdateSnapshot() {
  const auto& snapshot = service_->GetSnapshot();
  devices_.SetDevices(snapshot.devices);
  rooms_.SetRooms(snapshot.rooms);
  households_.SetHouseholds(snapshot.households);
}

void HomeViewModel::UpdateSelection() {
  filtered_rooms_.setHouseholdId(service_->GetCurrentHousehold());
  emit currentHouseholdChanged();
}
