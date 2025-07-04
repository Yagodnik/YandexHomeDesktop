#include "DevicesIcons.h"

#include "JsonLoader.h"

DevicesIcons::DevicesIcons(QObject *parent) {
  const auto data_opt = JsonLoader::Load<DevicesIconsData>(":/data/deviceIcons.json");
  if (!data_opt.has_value()) {
    qDebug() << "DevicesIcons::DevicesIcons: Failed to load deviceIcons.json";
    return;
  }

  data_ = data_opt.value();
}

QString DevicesIcons::GetIcon(const QString &device_type) const {
  return kBasePath + data_.icons.value(device_type, "other.svg").toString();
}
