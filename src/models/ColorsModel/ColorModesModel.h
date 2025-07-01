#pragma once

#include <QAbstractListModel>

#include "serialization/Serialization.h"

class ColorModesModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit ColorModesModel(QObject* parent = nullptr);

  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    ImageRole
  };

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
  JSON_STRUCT(ColorMode,
    (QString, id),
    (QString, name)
  );

  JSON_STRUCT(ColorModesFile,
    (QList<ColorMode>, color_modes)
  );

  const QString kModesFile = ":/data/colorModes.json";
  const QString kImagePrefix = "qrc:/images/modes/";

  QList<ColorMode> color_modes_;
};
