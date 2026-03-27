#pragma once

#include "serialization/Serialization.h"
#include <QObject>

class IOAuthSecrets : public QObject {
  Q_OBJECT
public:
  explicit IOAuthSecrets(QObject *parent = nullptr) : QObject(parent) {}

  JSON_STRUCT(AuthSecrets,
    (QString, auth_url),
    (QString, access_token_url),
    (QString, client_id),
    (QString, redirect_base),
    (int, redirect_port),
    (QStringList, scopes)
  );

  [[nodiscard]] virtual std::optional<AuthSecrets> GetAuthSecrets() const = 0;
};
