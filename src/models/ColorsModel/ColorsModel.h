#pragma once

#include <QAbstractListModel>

#include "serialization/Serialization.h"

class ColorsModel : public QAbstractListModel {
  Q_OBJECT
private:
  JSON_STRUCT(Rgb,
    (int, r),
    (int, g),
    (int, b)
  );

  JSON_STRUCT(Color,
    (QString, name),
    (Rgb, rgb)
  );

  JSON_STRUCT(Temperature,
    (QString, name),
    (Rgb, rgb),
    (int, temperature)
  );

  JSON_STRUCT(ColorsFile,
    (QList<Color>, colors),
    (QList<Temperature>, temperatures)
  );

  struct ColorData {
    QString name;
    Rgb rgb;
    std::optional<int> temperature;
  };

  const QString kColorsFile = ":/data/colors.json";

  QList<ColorData> colors_;

  void ProcessData(const ColorsFile& data);

  friend class ModelsTests;

public:
  explicit ColorsModel(QObject *parent = nullptr);
  explicit ColorsModel(const ColorsFile& test_data, QObject *parent = nullptr);

  enum Roles {
    IndexRole = Qt::UserRole + 1,
    NameRole,
    ColorRole,
    IsTemperatureRole,
    TemperatureRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};
