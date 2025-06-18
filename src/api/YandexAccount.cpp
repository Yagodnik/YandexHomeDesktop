#include "YandexAccount.h"
#include "RequestFactory.h"

YandexAccount::YandexAccount(TokenProvider token_provider, QObject *parent)
  : QObject(parent), token_provider_(std::move(token_provider)) {}

void YandexAccount::LoadData() {
  const auto request = RequestFactory::CreateBearer(kAccountInfoEndpoint, token_provider_());


}

const QString& YandexAccount::GetName() const {
  return name_;
}

QString YandexAccount::GetAvatarUrl() const {
  return kAvatarUrl.arg(avatar_id_);
}
