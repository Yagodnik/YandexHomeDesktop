#pragma once

#include <expected>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "model/Response.h"
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
  void ExecuteScenario(const QString& scenario_id, const std::function<void(const Response&)>& callback);

signals:
  void infoReceived(const UserInfo& info);
  void scenariosReceived(const QList<ScenarioObject>& scenarios);
  void errorReceived(const QString& error);
  void scenarioFinished(const QString& scenario_id);

private:
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";
  const QString kExecuteScenarioEndpoint = "https://api.iot.yandex.net/v1.0/scenarios/%1/actions";

  static std::expected<QJsonObject, QString> ParseResponseAsObject(const QByteArray& response);

  template<Serialization::Serializable T>
  void MakeGetRequest(const QString& endpoint, std::function<void(const T&)> callback) {
    const auto request = RequestFactory::Create(endpoint, token_provider_());

    QNetworkReply *reply = network_access_manager_.get(request);

    connect(reply, &QNetworkReply::finished, [reply, this, callback]() {
      ReplyGuard guard(reply);

      if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "YandexHomeApi::RequestInfo Network Error:" << reply->errorString();
        emit errorReceived(reply->errorString());

        return;
      }

      const auto response_bytes = reply->readAll();
      const auto response_object = ParseResponseAsObject(response_bytes);
      if (!response_object.has_value()) {
        qDebug() << "YandexHomeApi::RequestInfo Parsing Error:" << response_object.error();
        emit errorReceived(response_object.error());

        return;
      }

      const auto response = Serialization::From<T>(response_object.value());
      callback(response);
    });
  }

  template<Serialization::Serializable T>
  void MakePostRequest(const QString& endpoint, std::function<void(const T&)> callback) {
    const auto request = RequestFactory::Create(endpoint, token_provider_());

    QNetworkReply *reply = network_access_manager_.post(request, nullptr);

    connect(reply, &QNetworkReply::finished, [reply, this, callback]() {
      ReplyGuard guard(reply);

      if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "YandexHomeApi::RequestInfo Network Error:" << reply->errorString();
        emit errorReceived(reply->errorString());

        return;
      }

      const auto response_bytes = reply->readAll();
      const auto response_object = ParseResponseAsObject(response_bytes);
      if (!response_object.has_value()) {
        qDebug() << "YandexHomeApi::RequestInfo Parsing Error:" << response_object.error();
        emit errorReceived(response_object.error());

        return;
      }

      const auto response = Serialization::From<T>(response_object.value());
      callback(response);
    });
  }

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
