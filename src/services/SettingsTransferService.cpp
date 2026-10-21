#include "SettingsTransferService.h"
#include "utils/Settings.h"
#include <QFile>
#include <QSaveFile>

namespace {
SettingsTransferService::Error TransferError(Settings::ImportResult result) {
  switch (result) {
  case Settings::ImportResult::AccountMismatch:
    return SettingsTransferService::Error::AccountMismatch;
  case Settings::ImportResult::StorageError:
    return SettingsTransferService::Error::Storage;
  default:
    return SettingsTransferService::Error::InvalidData;
  }
}
} // namespace

SettingsTransferService::SettingsTransferService(Settings* settings, QObject* parent)
    : QObject(parent), settings_(settings) {}

std::expected<void, SettingsTransferService::Error>
SettingsTransferService::Export(const QString& accountId, const QUrl& file) {
  if (!file.isLocalFile())
    return std::unexpected(Error::File);
  const auto data = settings_->ExportSettings(accountId);
  if (!data)
    return std::unexpected(Error::Storage);
  QSaveFile output(file.toLocalFile());
  if (!output.open(QIODevice::WriteOnly) || output.write(*data) != data->size() || !output.commit())
    return std::unexpected(Error::File);
  return {};
}

std::expected<QByteArray, SettingsTransferService::Error>
SettingsTransferService::Read(const QString& accountId, const QUrl& file) {
  if (!file.isLocalFile())
    return std::unexpected(Error::File);
  QFile input(file.toLocalFile());
  constexpr qint64 maximumSize = 16 * 1024 * 1024;
  if (!input.open(QIODevice::ReadOnly) || input.size() > maximumSize)
    return std::unexpected(Error::File);
  const auto data = input.read(maximumSize + 1);
  if (input.error() != QFile::NoError || data.size() > maximumSize)
    return std::unexpected(Error::File);
  const auto result = settings_->CheckImport(accountId, data);
  if (result != Settings::ImportResult::Imported)
    return std::unexpected(TransferError(result));
  return data;
}

std::expected<void, SettingsTransferService::Error>
SettingsTransferService::Apply(const QString& accountId, const QByteArray& data) {
  const auto result = settings_->ImportSettings(accountId, data);
  if (result != Settings::ImportResult::Imported)
    return std::unexpected(TransferError(result));
  return {};
}
