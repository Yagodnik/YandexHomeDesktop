#include "KeychainTokenStore.h"

#include <QPointer>
#include "qtkeychain/keychain.h"

namespace {
// Preserve the existing identifiers so installed users retain their login.
const QString kService = "com.artemyagodnik.YandexHomeDesktop_Test";
const QString kKey = "test_secret";

AuthError Error(const QKeychain::Job* job) {
  const auto kind = job->error() == QKeychain::EntryNotFound ? AuthErrorKind::NotFound
    : job->error() == QKeychain::AccessDeniedByUser ? AuthErrorKind::Canceled
    : AuthErrorKind::Storage;
  return {kind, static_cast<quint32>(job->error()), job->errorString()};
}

template<typename Job, typename T, typename ReadResult>
void StartJob(Job* job, QObject* context, AuthResultHandler<T> handler, ReadResult read_result) {
  job->setKey(kKey);
  job->setAutoDelete(false);
  job->setInsecureFallback(false);
  const QPointer<QObject> guard(context);
  QObject::connect(job, &QKeychain::Job::finished, job,
    [job, guard, handler = std::move(handler), read_result] {
      if (guard) {
        if (job->error() == QKeychain::NoError) {
          handler(read_result(job));
        } else {
          handler(std::unexpected(Error(job)));
        }
      }
      job->deleteLater();
    });
  job->start();
}
}

void KeychainTokenStore::Read(QObject* context, AuthResultHandler<QString> handler) {
  StartJob(new QKeychain::ReadPasswordJob(kService, this), context, std::move(handler),
    [](QKeychain::ReadPasswordJob* job) -> AuthResult<QString> { return job->textData(); });
}

void KeychainTokenStore::Write(const QString& token, QObject* context, AuthResultHandler<void> handler) {
  auto* job = new QKeychain::WritePasswordJob(kService, this);
  job->setTextData(token);
  StartJob(job, context, std::move(handler),
    [](QKeychain::WritePasswordJob*) -> AuthResult<void> { return {}; });
}

void KeychainTokenStore::Delete(QObject* context, AuthResultHandler<void> handler) {
  StartJob(new QKeychain::DeletePasswordJob(kService, this), context, std::move(handler),
    [](QKeychain::DeletePasswordJob*) -> AuthResult<void> { return {}; });
}
