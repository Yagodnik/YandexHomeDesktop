#include "UnitsList.h"
#include "JsonLoader.h"
#include "TranslateCatalog.h"

namespace {
  JSON_STRUCT(UnitsListData,
    (QVariantMap, units)
  );
}

UnitsList::UnitsList(QObject *parent) : QObject(parent) {
  const auto temp = JsonLoader::Load<UnitsListData>(":/data/units.json");

  if (!temp.has_value()) {
    qCritical() << "TitlesList: Failed to load JSON data";
    return;
  }

  units_ = temp->units;
}

QString UnitsList::GetUnit(const QString &unit_name) const {
  return TranslateCatalog("DataUnits", units_.value(unit_name, "").toString());
}
