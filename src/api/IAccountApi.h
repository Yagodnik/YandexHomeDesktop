#pragma once

#include <QObject>

#include "ApiResult.h"

struct AccountInfo {
  QString display_name;
  QString default_avatar_id;
  QString default_email;
};

class IAccountApi {
public:
  virtual ~IAccountApi() = default;
  virtual void LoadData(QObject* context, ApiResultHandler<AccountInfo> handler) = 0;
};
