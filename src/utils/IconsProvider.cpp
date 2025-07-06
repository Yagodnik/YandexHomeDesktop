#include "IconsProvider.h"
#include "JsonLoader.h"

IconsProvider::IconsProvider(const QString& path, const QString& prefix, QObject *parent)
  : QObject(parent), base_path_(kBasePath + prefix + "/")
{
  const auto data_opt = JsonLoader::Load<IconsData>(path);
  if (!data_opt.has_value()) {
    qDebug() << "Icons: Failed to load deviceIcons.json";
    return;
  }

  data_ = data_opt.value();
}

QString IconsProvider::GetIcon(const QString &name) const {
  qDebug() << "Icons: Getting icon " << name;
  return base_path_ + data_.icons.value(name, "other.svg").toString();
}
