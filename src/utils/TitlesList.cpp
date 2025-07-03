#include "TitlesList.h"

TitlesList::TitlesList(QObject *parent) {
  const auto temp = JsonLoader::Load<TitlesListData>(":/data/instances.json");

  if (!temp.has_value()) {
    qDebug() << "TitlesList::TitlesList: Failed to load JSON data";
    return;
  }

  data_ = temp.value();
}

QString TitlesList::GetOnOffTitle(const QString &instance) const {
  return data_.on_off.value(instance, "").toString();
}

QString TitlesList::GetRangeTitle(const QString &instance) const {
  return data_.range.value(instance, "").toString();
}

QString TitlesList::GetToggleTitle(const QString &instance) const {
  return data_.toggle.value(instance, "").toString();
}

QString TitlesList::GetModeTitle(const QString &instance) const {
  return data_.mode.value(instance, "").toString();
}

QString TitlesList::GetTitle(const QString &name, const QString &instance) const {
  if (name == "on_off") {
    return GetOnOffTitle(instance);
  } else if (name == "range") {
    return GetRangeTitle(instance);
  } else if (name == "toggle") {
    return GetToggleTitle(instance);
  } else if (name == "mode") {
    return GetModeTitle(instance);
  }

  return "???";
}
