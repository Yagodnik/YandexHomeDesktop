#pragma once

#include <QObject>

#include "api/IHomeApi.h"

class HomeSnapshotLoader final : public QObject {
  Q_OBJECT
public:
  explicit HomeSnapshotLoader(IHomeApi* api, QObject* parent = nullptr);
  void Refresh();

signals:
  void loaded(const UserInfo& info);
  void failed(const QString& message);

private:
  IHomeApi* api_;
};
