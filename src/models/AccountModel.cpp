#include "AccountModel.h"

AccountModel::AccountModel(IAccountApi* api, QObject* parent) : QObject(parent), api_(api) {}

void AccountModel::LoadData() {
  loading_ = true;
  const auto generation = ++generation_;
  api_->LoadData(this, [this, generation](ApiResult<AccountInfo> result) {
    if (generation != generation_) { return; }
    loading_ = false;
    if (!result) {
      emit dataLoadingFailed();
      return;
    }

    name_ = result->display_name;
    avatar_id_ = result->default_avatar_id;
    email_ = result->default_email;
    account_id_ = result->id;
    emit dataLoaded();
  });
}

void AccountModel::EnsureLoaded() {
  if (account_id_.isEmpty() && !loading_) {
    LoadData();
  }
}

QString AccountModel::GetAccountId() const { return account_id_; }

void AccountModel::Reset() {
  ++generation_;
  name_.clear();
  avatar_id_.clear();
  email_.clear();
  account_id_.clear();
  loading_ = false;
  emit dataLoaded();
}

QString AccountModel::GetName() const {
  return name_;
}

QString AccountModel::GetAvatarUrl() const {
  if (avatar_id_.isEmpty()) {
    return "qrc:/images/icon.png";
  }
  return kAvatarUrl.arg(avatar_id_);
}

QString AccountModel::GetEmail() const {
  return email_;
}
