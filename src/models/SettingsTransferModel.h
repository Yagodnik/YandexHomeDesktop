#pragma once

#include "services/SettingsTransferService.h"
#include <QObject>

class AccountModel;

class SettingsTransferModel final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ IsAvailable NOTIFY changed)
  Q_PROPERTY(bool importPending READ IsImportPending NOTIFY changed)
  Q_PROPERTY(QString status READ GetStatus NOTIFY changed)
  Q_PROPERTY(bool failed READ HasFailed NOTIFY changed)
public:
  SettingsTransferModel(SettingsTransferService* service, AccountModel* account,
                        QObject* parent = nullptr);
  bool IsAvailable() const;
  bool IsImportPending() const;
  QString GetStatus() const;
  bool HasFailed() const;
  void SetAccountId(const QString& id);
  Q_INVOKABLE bool Export(const QUrl& file);
  Q_INVOKABLE bool PrepareImport(const QUrl& file);
  Q_INVOKABLE bool ConfirmImport();
  Q_INVOKABLE void CancelImport();

private:
  bool Fail(SettingsTransferService::Error error);
  bool RequireAccount();
  SettingsTransferService* service_;
  QString account_id_;
  QByteArray pending_;
  QString status_;
  bool failed_ = false;
signals:
  void changed();
};
