#include "AccountModel.h"

AccountModel::AccountModel(IAccountApi* api, QObject* parent) : QObject(parent), api_(api) {}

void AccountModel::LoadData() {
  const auto generation = ++generation_;
  api_->LoadData(this, [this, generation](ApiResult<AccountInfo> result) {
    if (generation != generation_) { return; }
    if (!result) {
      emit dataLoadingFailed();
      return;
    }

    name_ = result->display_name;
    avatar_id_ = result->default_avatar_id;
    email_ = result->default_email;
    emit dataLoaded();
  });
}

void AccountModel::Reset() {
  ++generation_;
  name_.clear();
  avatar_id_.clear();
  email_.clear();
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
