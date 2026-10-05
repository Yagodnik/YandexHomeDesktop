#include "HomeService.h"

#include <algorithm>
#include <QDebug>

HomeService::HomeService(IHomeApi* api, QObject* parent) : QObject(parent), api_(api) {}

HomeService::LoadState HomeService::GetLoadState() const { return state_; }
const UserInfo& HomeService::GetSnapshot() const { return snapshot_; }
QString HomeService::GetCurrentHousehold() const { return current_household_; }

QString HomeService::GetCurrentHouseholdName() const {
  for (const auto& household : snapshot_.households) {
    if (household.id == current_household_) {
      return household.name;
    }
  }
  return {};
}

void HomeService::EnsureLoaded() {
  if (state_ == LoadState::NotLoaded) {
    Refresh();
  }
}

void HomeService::Refresh() {
  if (state_ == LoadState::Loading) {
    return;
  }
  const auto request = QUuid::createUuid();
  request_ = request;
  SetLoadState(LoadState::Loading);
  api_->GetUserInfo(this, [this, request](ApiResult<UserInfo> result) {
    if (request_ != request) {
      return;
    }
    request_.reset();
    if (!result) {
      qWarning() << "HomeService: Loading failed:" << result.error().message;
      SetLoadState(LoadState::Error);
      return;
    }

    const auto previous_id = current_household_;
    const auto previous_name = GetCurrentHouseholdName();
    snapshot_ = std::move(*result);
    const auto selected = std::find_if(snapshot_.households.cbegin(), snapshot_.households.cend(),
      [this](const HouseholdObject& household) { return household.id == current_household_; });
    if (selected == snapshot_.households.cend()) {
      current_household_ = snapshot_.households.isEmpty() ? QString{} : snapshot_.households.first().id;
    }
    emit snapshotChanged();
    if (previous_id != current_household_ || previous_name != GetCurrentHouseholdName()) {
      emit currentHouseholdChanged();
    }
    SetLoadState(LoadState::Ready);
  });
}

void HomeService::SelectHousehold(const QString& id) {
  if (id == current_household_) {
    return;
  }
  const auto household = std::find_if(snapshot_.households.cbegin(), snapshot_.households.cend(),
    [&id](const HouseholdObject& item) { return item.id == id; });
  if (household == snapshot_.households.cend()) {
    return;
  }
  current_household_ = id;
  emit currentHouseholdChanged();
}

void HomeService::Reset() {
  request_.reset();
  snapshot_ = {};
  const bool had_selection = !current_household_.isEmpty();
  current_household_.clear();
  emit snapshotChanged();
  if (had_selection) {
    emit currentHouseholdChanged();
  }
  SetLoadState(LoadState::NotLoaded);
}

void HomeService::SetLoadState(LoadState state) {
  if (state_ != state) {
    state_ = state;
    emit loadStateChanged();
  }
}
