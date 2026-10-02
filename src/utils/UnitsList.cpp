#include "UnitsList.h"
#include "TranslateCatalog.h"

UnitsList::UnitsList(QObject *parent) : QObject(parent) {
  const auto temp = JsonLoader::Load<UnitsListData>(":/data/units.json");

  if (!temp.has_value()) {
    qCritical() << "TitlesList: Failed to load JSON data";
    return;
  }

  data_ = temp.value();
}

QString UnitsList::GetUnit(const QString &unit_name) const {
  return TranslateCatalog("DataUnits", data_.units.value(unit_name, "").toString());
}
