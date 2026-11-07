#pragma once

#include "yh/appservices_export.h"
#include <QObject>
#include <QUrl>
#include <expected>

class Settings;

class APPSERVICES_EXPORT SettingsTransferService final : public QObject {
public:
  enum class Error { File, InvalidData, AccountMismatch, Storage };
  explicit SettingsTransferService(Settings* settings, QObject* parent = nullptr);
  std::expected<void, Error> Export(const QString& accountId, const QUrl& file);
  std::expected<QByteArray, Error> Read(const QString& accountId, const QUrl& file);
  std::expected<void, Error> Apply(const QString& accountId, const QByteArray& data);

private:
  Settings* settings_;
};
