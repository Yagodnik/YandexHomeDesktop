#include "SettingsTransferModel.h"
#include "AccountModel.h"

SettingsTransferModel::SettingsTransferModel(SettingsTransferService* service,
                                             AccountModel* account, QObject* parent)
    : QObject(parent), service_(service) {
  if (account) {
    connect(account, &AccountModel::dataLoaded, this,
            [this, account] { SetAccountId(account->GetAccountId()); });
    SetAccountId(account->GetAccountId());
  }
}
bool SettingsTransferModel::IsAvailable() const {
  return !account_id_.isEmpty();
}
bool SettingsTransferModel::IsImportPending() const {
  return !pending_.isEmpty();
}
QString SettingsTransferModel::GetStatus() const {
  return status_;
}
bool SettingsTransferModel::HasFailed() const {
  return failed_;
}

void SettingsTransferModel::SetAccountId(const QString& id) {
  if (id == account_id_)
    return;
  account_id_ = id;
  pending_.clear();
  status_.clear();
  failed_ = false;
  emit changed();
}
bool SettingsTransferModel::RequireAccount() {
  if (IsAvailable())
    return true;
  status_ = tr("Дождитесь загрузки аккаунта.");
  failed_ = true;
  emit changed();
  return false;
}
bool SettingsTransferModel::Fail(SettingsTransferService::Error error) {
  switch (error) {
  case SettingsTransferService::Error::File:
    status_ = tr("Не удалось открыть или сохранить файл. Проверьте путь и права доступа.");
    break;
  case SettingsTransferService::Error::InvalidData:
    status_ = tr("Файл содержит некорректные настройки или неподдерживаемую версию.");
    break;
  case SettingsTransferService::Error::AccountMismatch:
    status_ = tr("Файл настроек принадлежит другому аккаунту.");
    break;
  case SettingsTransferService::Error::Storage:
    status_ = tr("Не удалось прочитать или сохранить настройки.");
    break;
  }
  failed_ = true;
  emit changed();
  return false;
}
bool SettingsTransferModel::Export(const QUrl& file) {
  if (!RequireAccount())
    return false;
  const auto result = service_->Export(account_id_, file);
  if (!result)
    return Fail(result.error());
  status_ = tr("Настройки сохранены в файл.");
  failed_ = false;
  emit changed();
  return true;
}
bool SettingsTransferModel::PrepareImport(const QUrl& file) {
  pending_.clear();
  if (!RequireAccount())
    return false;
  const auto result = service_->Read(account_id_, file);
  if (!result)
    return Fail(result.error());
  pending_ = *result;
  status_.clear();
  failed_ = false;
  emit changed();
  return true;
}
bool SettingsTransferModel::ConfirmImport() {
  if (!RequireAccount() || pending_.isEmpty())
    return false;
  const auto result = service_->Apply(account_id_, pending_);
  pending_.clear();
  if (!result)
    return Fail(result.error());
  status_ = tr("Настройки загружены.");
  failed_ = false;
  emit changed();
  return true;
}
void SettingsTransferModel::CancelImport() {
  pending_.clear();
  emit changed();
}
