#include "HomeViewModel.h"
#include "AccountModel.h"
#include "utils/Settings.h"
#include <algorithm>

HomeViewModel::HomeViewModel(HomeService* service, QObject* parent)
  : HomeViewModel(service, nullptr, nullptr, parent) {}

HomeViewModel::HomeViewModel(HomeService* service, Settings* settings, AccountModel* account, QObject* parent)
  : QObject(parent), service_(service), settings_(settings), account_(account),
    devices_(this), favorites_(this), rooms_(this), households_(this), filtered_rooms_(this) {
  filtered_rooms_.setSourceModel(&rooms_);
  connect(service_, &HomeService::loadStateChanged, this, &HomeViewModel::stateChanged);
  connect(service_, &HomeService::snapshotChanged, this, &HomeViewModel::UpdateSnapshot);
  connect(service_, &HomeService::currentHouseholdChanged, this, &HomeViewModel::UpdateSelection);
  if (account_) {
    connect(account_, &AccountModel::dataLoaded, this, &HomeViewModel::UpdateAccount);
  }
  UpdateAccount();
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
DevicesModel* HomeViewModel::GetFavorites() { return &favorites_; }
bool HomeViewModel::AreFavoritesCollapsed() const { return favorites_collapsed_; }
bool HomeViewModel::ArePreferencesAvailable() const { return settings_ && !account_id_.isEmpty(); }
RoomsFilterModel* HomeViewModel::GetRooms() { return &filtered_rooms_; }
HouseholdsModel* HomeViewModel::GetHouseholds() { return &households_; }
QString HomeViewModel::GetCurrentHousehold() const { return service_->GetCurrentHousehold(); }
QString HomeViewModel::GetCurrentHouseholdName() const { return service_->GetCurrentHouseholdName(); }
void HomeViewModel::EnsureLoaded() {
  if (account_) { account_->EnsureLoaded(); }
  service_->EnsureLoaded();
}
void HomeViewModel::Refresh() {
  if (account_) { account_->EnsureLoaded(); }
  service_->Refresh();
}
void HomeViewModel::SelectHousehold(const QString& id) { service_->SelectHousehold(id); }

void HomeViewModel::UpdateSnapshot() {
  const auto& snapshot = service_->GetSnapshot();
  devices_.SetDevices(snapshot.devices);
  rooms_.SetRooms(snapshot.rooms);
  households_.SetHouseholds(snapshot.households);
  UpdateFavorites();
}

void HomeViewModel::UpdateSelection() {
  filtered_rooms_.setHouseholdId(service_->GetCurrentHousehold());
  UpdateFavorites();
  emit currentHouseholdChanged();
}

void HomeViewModel::UpdateAccount() {
  const auto id = account_ ? account_->GetAccountId() : QString{};
  if (account_id_ == id) { return; }
  account_id_ = id;
  favorite_ids_ = settings_ ? settings_->GetFavoriteDevices(id) : QStringList{};
  collapsed_room_ids_ = settings_ ? settings_->GetCollapsedRooms(id) : QStringList{};
  const bool favorites_collapsed = settings_ && settings_->GetFavoritesCollapsed(id);
  if (favorites_collapsed_ != favorites_collapsed) {
    favorites_collapsed_ = favorites_collapsed;
    emit favoritesCollapsedChanged();
  }
  favorite_ids_.removeDuplicates();
  collapsed_room_ids_.removeDuplicates();
  devices_.SetFavoriteDevices(favorite_ids_);
  rooms_.SetCollapsedRooms(collapsed_room_ids_);
  UpdateFavorites();
  emit preferencesAvailableChanged();
}

void HomeViewModel::UpdateFavorites() {
  QList<DeviceObject> items;
  const auto& devices = service_->GetSnapshot().devices;
  for (const auto& id : favorite_ids_) {
    const auto device = std::find_if(devices.cbegin(), devices.cend(), [this, &id](const DeviceObject& item) {
      return item.id == id && item.household_id == GetCurrentHousehold();
    });
    if (device != devices.cend()) { items.append(*device); }
  }
  favorites_.SetFavoriteDevices(favorite_ids_);
  favorites_.SetDevices(items);
}

void HomeViewModel::SetDeviceFavorite(const QString& id, bool favorite) {
  if (!ArePreferencesAvailable() || favorite_ids_.contains(id) == favorite) { return; }
  const auto& devices = service_->GetSnapshot().devices;
  if (std::none_of(devices.cbegin(), devices.cend(), [&id](const DeviceObject& item) { return item.id == id; })) {
    return;
  }
  if (favorite) { favorite_ids_.append(id); } else { favorite_ids_.removeAll(id); }
  settings_->SetFavoriteDevices(account_id_, favorite_ids_);
  devices_.SetFavoriteDevices(favorite_ids_);
  UpdateFavorites();
}

void HomeViewModel::SetRoomCollapsed(const QString& id, bool collapsed) {
  if (!ArePreferencesAvailable() || collapsed_room_ids_.contains(id) == collapsed) { return; }
  const auto& rooms = service_->GetSnapshot().rooms;
  if (std::none_of(rooms.cbegin(), rooms.cend(), [&id](const RoomObject& item) { return item.id == id; })) {
    return;
  }
  if (collapsed) { collapsed_room_ids_.append(id); } else { collapsed_room_ids_.removeAll(id); }
  settings_->SetCollapsedRooms(account_id_, collapsed_room_ids_);
  rooms_.SetCollapsedRooms(collapsed_room_ids_);
}

void HomeViewModel::SetFavoritesCollapsed(bool collapsed) {
  if (!ArePreferencesAvailable() || favorites_collapsed_ == collapsed) { return; }
  favorites_collapsed_ = collapsed;
  settings_->SetFavoritesCollapsed(account_id_, collapsed);
  emit favoritesCollapsedChanged();
}
