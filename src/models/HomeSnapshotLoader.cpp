#include "HomeSnapshotLoader.h"

HomeSnapshotLoader::HomeSnapshotLoader(IHomeApi* api, QObject* parent)
  : QObject(parent), api_(api) {}

void HomeSnapshotLoader::Refresh() {
  api_->GetUserInfo(this, [this](ApiResult<UserInfo> result) {
    if (result) {
      emit loaded(*result);
    } else {
      emit failed(result.error().message);
    }
  });
}
