#pragma once

#include "serialization/Serialization.h"

class IconsProvider : public QObject {
  Q_OBJECT
public:
  explicit IconsProvider(const QString& path, const QString& prefix, QObject *parent);

  Q_INVOKABLE [[nodiscard]] QString GetIcon(const QString& name) const;

private:
  JSON_STRUCT(IconsData,
    (QVariantMap, icons)
  );

  const QString kBasePath = "qrc:/images/";
  const QString base_path_;
  IconsData data_;
};
