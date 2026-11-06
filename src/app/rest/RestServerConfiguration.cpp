#include "RestServerConfiguration.h"
#include "serialization/Serialization.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUrl>
#include <algorithm>
#include <cmath>

namespace {
constexpr qsizetype MaxConfigurationBytes = 64 * 1024;
class RestConfigurationMessages {
  Q_DECLARE_TR_FUNCTIONS(RestConfigurationMessages)
};

JSON_ENUMERATION(ClientAuthMode, ("none", None), ("bearer", Bearer));
JSON_STRUCT(RestConfigurationFields, (QString, host), (double, port), (QString, oauth_token),
            (ClientAuthMode, auth_mode), (QString, client_token));
} // namespace

QString RestServerConfiguration::Url(quint16 port) const {
  QUrl url;
  url.setScheme("http");
  url.setHost(host.toString());
  url.setPort(port);
  url.setPath("/v1");
  return url.toString();
}

std::expected<RestServerConfiguration, QString> LoadRestServerConfiguration(const QString& path) {
  RestServerConfiguration configuration;
  if (path.isEmpty()) {
    return configuration;
  }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(RestConfigurationMessages::tr("Cannot read REST configuration file."));
  }
  const auto bytes = file.read(MaxConfigurationBytes + 1);
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(bytes, &error);
  if (bytes.size() > MaxConfigurationBytes || error.error != QJsonParseError::NoError ||
      !document.isObject()) {
    return std::unexpected(RestConfigurationMessages::tr(
        "REST configuration must be a JSON object no larger than 64 KiB."));
  }
  const auto object = document.object();
  // From leaves absent members untouched; supply every field before decoding.
  auto fields_object =
      Serialization::To(RestConfigurationFields{.host = configuration.host.toString(),
                                                .port = 0,
                                                .oauth_token = {},
                                                .auth_mode = ClientAuthMode::None,
                                                .client_token = {}});
  for (auto field = object.begin(); field != object.end(); ++field) {
    if (!fields_object.contains(field.key())) {
      return std::unexpected(RestConfigurationMessages::tr("Unknown REST configuration field."));
    }
    fields_object.insert(field.key(), field.value());
  }
  const auto fields = Serialization::From<RestConfigurationFields>(fields_object);
  if (object.contains("host")) {
    const auto value = object["host"];
    const auto& host = fields.host;
    if (host.compare("localhost", Qt::CaseInsensitive) == 0) {
      configuration.host = QHostAddress::LocalHost;
    } else if (!value.isString() || !configuration.host.setAddress(host)) {
      return std::unexpected(
          RestConfigurationMessages::tr("REST host must be an IPv4/IPv6 address or localhost."));
    }
  }
  if (object.contains("port")) {
    const auto value = object["port"];
    const auto port = fields.port;
    if (!value.isDouble() || port < 1 || port > 65535 || std::floor(port) != port) {
      return std::unexpected(
          RestConfigurationMessages::tr("REST port must be an integer from 1 to 65535."));
    }
    configuration.port = static_cast<quint16>(port);
  }
  if (fields.auth_mode == ClientAuthMode::Unknown) {
    return std::unexpected(RestConfigurationMessages::tr("auth_mode must be bearer or none."));
  }
  if (fields.auth_mode == ClientAuthMode::Bearer) {
    static const QRegularExpression token_pattern(QStringLiteral(R"(\A[A-Za-z0-9._~+/-]+=*\z)"));
    if (!object["client_token"].isString() ||
        !token_pattern.match(fields.client_token).hasMatch()) {
      return std::unexpected(
          RestConfigurationMessages::tr("Bearer authentication requires a valid client_token."));
    }
    if (fields.client_token == fields.oauth_token) {
      return std::unexpected(
          RestConfigurationMessages::tr("client_token must differ from oauth_token."));
    }
    configuration.client_token = fields.client_token.toLatin1();
  } else if (object.contains("client_token") || !configuration.host.isLoopback()) {
    return std::unexpected(RestConfigurationMessages::tr(
        "REST requires auth_mode bearer to use a client_token or a non-loopback host."));
  }
  if (object.contains("oauth_token")) {
    const auto value = object["oauth_token"];
    configuration.oauth_token = fields.oauth_token;
    if (!value.isString() || configuration.oauth_token.isEmpty() ||
        std::any_of(configuration.oauth_token.begin(), configuration.oauth_token.end(),
                    [](QChar character) {
                      return character.isSpace() || character.unicode() < 0x20 ||
                             character.unicode() == 0x7f;
                    })) {
      return std::unexpected(RestConfigurationMessages::tr(
          "oauth_token must be a nonempty string without whitespace or control characters."));
    }
  }
  return configuration;
}
