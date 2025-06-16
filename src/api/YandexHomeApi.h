#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "model/UserInfo.h"

struct RequestFactory {
  inline static const QString kBearer = "Bearer ";

  static QNetworkRequest Create(const QString& endpoint, const QString& token);
};

class YandexHomeApi final : public QObject {
  Q_OBJECT
public:
  using TokenProvider = std::function<QString()>;
  explicit YandexHomeApi(TokenProvider token_provider, QObject *parent = nullptr);

  Q_INVOKABLE void RequestInfo();
  Q_INVOKABLE void GetScenarios();
  Q_INVOKABLE void ExecuteScenario(const QString& scenario_id);

signals:
  void infoReceived(const UserInfo& info);
  void scenariosReceived(const QList<ScenarioObject>& scenarios);
  void errorReceived(const QString& error);

private:
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";

  template<Serialization::Serializable T>
  void MakeApiRequest(const QString& endpoint, std::function<void(const T&)> callback) {
    const auto request = RequestFactory::Create(endpoint, token_provider_());

    QNetworkReply *reply = network_access_manager_.get(request);

    connect(reply, &QNetworkReply::finished, [reply, this, callback]() {
      ReplyGuard guard(reply);

      if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "YandexHomeApi::RequestInfo Error:" << reply->errorString();

        emit errorReceived(reply->errorString());

        return;
      }

      const QByteArray response_bytes = reply->readAll();

      QJsonParseError json_error;
      const QJsonDocument json_response = QJsonDocument::fromJson(
        response_bytes, &json_error);

      if (json_error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << json_error.errorString();
        emit errorReceived(json_error.errorString());
        return;
      }

      const QJsonObject response_object = json_response.object();
      const auto response = Serialization::From<T>(response_object);

      callback(response);
    });
  }

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
