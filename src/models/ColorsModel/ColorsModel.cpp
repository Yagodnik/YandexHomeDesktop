#include "ColorsModel.h"
#include "utils/JsonLoader.h"
#include <QColor>

void ColorsModel::ProcessData(const ColorsFile &data) {
  for (const auto& candidate : data.colors) {
    colors_.push_back({
      candidate.name,
      candidate.rgb,
      std::nullopt
    });
  }

  for (const auto& candidate : data.temperatures) {
    colors_.push_back({
      candidate.name,
      candidate.rgb,
      candidate.temperature
    });
  }
}

ColorsModel::ColorsModel(QObject *parent) : QAbstractListModel(parent)
{
  const auto& colors_file = JsonLoader::Load<ColorsFile>(kColorsFile);

  if (!colors_file.has_value()) {
    qCritical() << "ColorsModel: Failed to load colors/temperatures file";
    return;
  }

  const auto& data = colors_file.value();

  ProcessData(data);
}

ColorsModel::ColorsModel(const ColorsFile &test_data, QObject *parent)
  : QAbstractListModel(parent)
{
  ProcessData(test_data);
}

int ColorsModel::rowCount(const QModelIndex &parent) const {
  return colors_.size();
}

QVariant ColorsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= colors_.size()) {
    return {};
  }

  const auto& color = colors_.at(index.row());
  const auto& color_data = color.rgb;

  switch (role) {
    case IndexRole:
      return index.row();
    case NameRole:
      return color.name;
    case ColorRole:
      return QColor(
        color_data.r,
        color_data.g,
        color_data.b
      );
    case IsTemperatureRole:
      return color.temperature.has_value();
    case TemperatureRole:
      if (color.temperature.has_value()) {
        return color.temperature.value();
      }

      qInfo() << "ColorsModel: Accessing temperature, while color is not temperature!";
      return {};
    default:
      return {};
  }
}

QHash<int, QByteArray> ColorsModel::roleNames() const {
  return {
    { IndexRole, "colorIndex" },
    { NameRole, "name" },
    { ColorRole, "color" },
    { IsTemperatureRole, "isTemperature" },
    { TemperatureRole, "temperature" }
  };
}
