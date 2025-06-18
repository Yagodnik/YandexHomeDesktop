#include "YandexAccount.h"

#include <QNetworkReply>

#include "RequestFactory.h"

YandexAccount::YandexAccount(TokenProvider token_provider, QObject *parent)
  : QObject(parent), token_provider_(std::move(token_provider)) {}

void YandexAccount::LoadData() {
  const auto request = RequestFactory::CreateBearer(kAccountInfoEndpoint, token_provider_());

  auto reply = network_manager_.get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() != QNetworkReply::NoError) {
      emit dataLoadingFailed();
      reply->deleteLater();
      return;
    }

    QByteArray response = reply->readAll();

    QJsonDocument document = QJsonDocument::fromJson(response);
    QJsonObject root = document.object();

    const auto data = Serialization::From<AccountInfo>(root);

    name_ = data.display_name;
    avatar_id_ = data.default_avatar_id;

    emit dataLoaded();

    reply->deleteLater();
  });
}

QString YandexAccount::GetName() const {
  return name_;
}

QString YandexAccount::GetAvatarUrl() const {
  return kAvatarUrl.arg(avatar_id_);
}
