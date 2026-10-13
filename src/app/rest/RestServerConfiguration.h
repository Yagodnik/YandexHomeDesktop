#pragma once

#include <QHostAddress>
#include <QString>
#include <expected>
#include <optional>

struct RestServerConfiguration {
  QHostAddress host{QHostAddress::LocalHost};
  std::optional<quint16> port;
  QString oauth_token;
  QString Url(quint16 port) const;
};

// Reads runtime settings only; never persists or reports the supplied token.
std::expected<RestServerConfiguration, QString> LoadRestServerConfiguration(const QString& path);
