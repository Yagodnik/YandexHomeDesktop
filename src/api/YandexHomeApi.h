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
  void ExecuteScenario(const QString& scenario_id, const QVariant& user_data = QVariant());

signals:
  void infoReceived(const UserInfo& info);
  void infoReceivingFailed(const QString& message);

  void scenariosReceivedSuccessfully(const QList<ScenarioObject>& scenarios);
  void scenariosReceivingFailed(const QString& message);

  void errorReceived(const QString& error);

  void scenarioExecutionFinished(const QString& scenario_id);
  void scenarioExecutionFinishedSuccessfully(const QString& scenario_id, const QVariant& user_data);
  void scenarioExecutionFailed(const QString& scenario_id, const QVariant& user_data);

private:
  using ReplyGuard = QScopedPointer<QNetworkReply, QScopedPointerDeleteLater>;

  const QString kInfoEndpoint = "https://api.iot.yandex.net/v1.0/user/info";
  const QString kExecuteScenarioEndpoint = "https://api.iot.yandex.net/v1.0/scenarios/%1/actions";

  static std::expected<QJsonObject, QString> ParseResponseAsObject(const QByteArray& response);

  template<Serialization::Serializable T>
  void MakeGetRequest(
    const QString& endpoint,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback
  ) {
    const auto request = RequestFactory::Create(endpoint, token_provider_());

    QNetworkReply *reply = network_access_manager_.get(request);

    connect(reply, &QNetworkReply::finished, [reply, this, ok_callback, error_callback]() {
      ReplyGuard guard(reply);

      if (reply->error() != QNetworkReply::NoError) {
        error_callback(reply->errorString());
        return;
      }

      const auto response_bytes = reply->readAll();
      const auto response_object = ParseResponseAsObject(response_bytes);

      if (!response_object.has_value()) {
        error_callback(response_object.error());
        return;
      }

      const auto response = Serialization::From<T>(response_object.value());
      ok_callback(response);
    });
  }

  template<Serialization::Serializable T>
  void MakePostRequest(
    const QString& endpoint,
    std::function<void(const T&)> ok_callback,
    std::function<void(const QString&)> error_callback
  ) {
    const auto request = RequestFactory::Create(endpoint, token_provider_());

    QNetworkReply *reply = network_access_manager_.post(request, nullptr);

    connect(reply, &QNetworkReply::finished, [reply, this, ok_callback, error_callback]() {
      ReplyGuard guard(reply);

      if (reply->error() != QNetworkReply::NoError) {
        error_callback(reply->errorString());
        return;
      }

      const auto response_bytes = reply->readAll();
      const auto response_object = ParseResponseAsObject(response_bytes);

      if (!response_object.has_value()) {
        error_callback(response_object.error());
        return;
      }

      const auto response = Serialization::From<T>(response_object.value());
      ok_callback(response);
    });
  }

  QNetworkAccessManager network_access_manager_;
  TokenProvider token_provider_;
};
