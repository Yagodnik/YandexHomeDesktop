#pragma once

#include "serialization/Serialization.h"

class DevicesIcons : public QObject {
  Q_OBJECT
public:
  explicit DevicesIcons(QObject *parent);

  Q_INVOKABLE [[nodiscard]] QString GetIcon(const QString& device_type) const;

private:
  JSON_STRUCT(DevicesIconsData,
    (QVariantMap, icons)
  );

  const QString kBasePath = "qrc:/images/devices/";
  DevicesIconsData data_;
};
