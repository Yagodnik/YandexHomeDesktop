#pragma once

#include "JsonLoader.h"

class TitlesProvider : public QObject {
  Q_OBJECT
public:
  explicit TitlesProvider(const QString& path, QObject *parent = nullptr);

  [[nodiscard]] QString GetTitle(const QString& name, const QString& instance) const;

private:
  QJsonObject raw_data_;
};
