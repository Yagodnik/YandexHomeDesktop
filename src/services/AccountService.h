#pragma once

#include "yh/appservices_export.h"

#include "api/IAccountApi.h"

class AccountService final {
public:
  explicit AccountService(IAccountApi* api) : api_(api) {}
  void ReadAccount(QObject* context, ApiResultHandler<AccountInfo> handler) {
    api_->LoadData(context, std::move(handler));
  }
private:
  IAccountApi* api_;
};
