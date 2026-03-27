#include "QtKeyChainSecretsStorage.h"
#include <QDebug>

QtKeyChainSecretsStorage::QtKeyChainSecretsStorage(
  const QString &appName,
  const QString& secureKey,
  QObject *parent
) :
  ISecretsStorage(parent),
  appName_(appName),
  secureKey_(secureKey)
{
  qDebug() << "Init: " << appName_ << " " << secureKey_;
}

void QtKeyChainSecretsStorage::TryWrite(const QString &token, WriteCallback callback) {
  auto *job = new QKeychain::WritePasswordJob(appName_);
  job->setKey(secureKey_);
  job->setTextData(token);

  // TODO: Maybe pass by ref...?
  connect(job, &QKeychain::Job::finished, [job, callback] {
    if (job->error() != QKeychain::NoError) {
      auto error = ISecretsStorage::Error {
        .errorCode = job->error(),
        .errorText = job->errorString()
      };

      callback(error);
    } else {
      callback(std::nullopt);
    }

    job->deleteLater();
  });

  job->start();
}

void QtKeyChainSecretsStorage::TryRead(ReadCallback callback) {
  auto *job = new QKeychain::ReadPasswordJob(appName_);
  job->setKey(secureKey_);

  connect(job, &QKeychain::Job::finished, [this, job, callback] {
    if (job->error() != QKeychain::NoError) {
      auto error = ISecretsStorage::Error {
        .errorCode = job->error(),
        .errorText = job->errorString()
      };

      callback(std::unexpected(error));
    } else {
      callback(job->textData());
    }

    job->deleteLater();
  });

  job->start();
}

void QtKeyChainSecretsStorage::TryDelete(DeleteCallback callback) {
  auto *job = new QKeychain::DeletePasswordJob(appName_);
  job->setKey(secureKey_);

  connect(job, &QKeychain::Job::finished, [job, callback] {
    if (job->error() != QKeychain::NoError) {
      auto error = ISecretsStorage::Error {
        .errorCode = job->error(),
        .errorText = job->errorString()
      };

      callback(error);
    } else {
      callback(std::nullopt);
    }

    job->deleteLater();
  });

  job->start();
}
