#pragma once

#include "yh/apprest_export.h"

#include <expected>

#include <QHttpServerRequest>

#include "RestReply.h"
#include "api/model/Capabilites.h"

struct RestAction {
  CapabilityType type;
  QVariantMap state;
};

struct RestActionError {
  RestReply::StatusCode status;
  QString code;
  QString message;
};

// JSON schema validation belongs to the HTTP adapter; device rules do not.
APPREST_EXPORT std::expected<RestAction, RestActionError> ParseRestAction(const QHttpServerRequest& request);
