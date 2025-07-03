#include "UnitsList.h"

UnitsList::UnitsList(QObject *parent) : QObject(parent) {
  const auto temp = JsonLoader::Load<UnitsListData>(":/data/units.json");

  if (!temp.has_value()) {
    qDebug() << "TitlesList::TitlesList: Failed to load JSON data";
    return;
  }

  data_ = temp.value();
}

QString UnitsList::GetUnit(const QString &unit_name) const {
  return data_.units.value(unit_name, "").toString();
}
