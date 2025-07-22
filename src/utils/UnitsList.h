#pragma once

#include "utils/JsonLoader.h"

class UnitsList : public QObject {
  Q_OBJECT
public:
  explicit UnitsList(QObject *parent = nullptr);

  [[nodiscard]] QString GetUnit(const QString &unit_name) const;

private:
  JSON_STRUCT(UnitsListData,
    (QVariantMap, units)
  );

  UnitsListData data_;
};
