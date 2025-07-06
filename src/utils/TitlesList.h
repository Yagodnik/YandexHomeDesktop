#pragma once

#include "JsonLoader.h"

class TitlesList : public QObject {
  Q_OBJECT
public:
  explicit TitlesList(QObject *parent = nullptr);

  [[nodiscard]] QString GetOnOffTitle(const QString& instance) const;
  [[nodiscard]] QString GetRangeTitle(const QString& instance) const;
  [[nodiscard]] QString GetToggleTitle(const QString& instance) const;
  [[nodiscard]] QString GetModeTitle(const QString& instance) const;
  [[nodiscard]] QString GetFloatTitle(const QString& instance) const;
  [[nodiscard]] QString GetEventTitle(const QString& instance) const;
  [[nodiscard]] QString GetTitle(const QString& name, const QString& instance) const;

private:
  JSON_STRUCT(TitlesListData,
    (QVariantMap, on_off),
    (QVariantMap, range),
    (QVariantMap, toggle),
    (QVariantMap, mode),
    (QVariantMap, float_),
    (QVariantMap, event)
  );

  TitlesListData data_;
};
