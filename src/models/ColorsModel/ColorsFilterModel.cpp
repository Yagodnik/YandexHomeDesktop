#include "ColorsFilterModel.h"

#include "ColorsModel.h"

ColorsFilterModel::ColorsFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent), min_(0), max_(9500) {
  QSortFilterProxyModel::sort(0, Qt::AscendingOrder);
}

void ColorsFilterModel::SetMin(const int min) {
  if (min_ == min) {
    return;
  }

  min_ = min;
  emit minChanged();
}

void ColorsFilterModel::SetMax(const int max) {
  if (max_ == max) {
    return;
  }

  max_ = max;
  emit maxChanged();
}

void ColorsFilterModel::SetSupportsColors(const bool value) {
  if (IsSupportsColors() != value) {
    supports_colors_ = value;
    emit supportsColorsChanged();
  }
}

void ColorsFilterModel::SetSupportsTemperature(const bool value) {
  if (IsSupportsTemperature() != value) {
    supports_temperature_ = value;
    emit supportsTemperatureChanged();
  }
}

void ColorsFilterModel::SetSupportsScenes(const bool value) {
  if (IsSupportsScenes() != value) {
    supports_scenes_ = value;
    emit supportsScenesChanged();
  }
}

int ColorsFilterModel::GetMin() const {
  return min_;
}

int ColorsFilterModel::GetMax() const {
  return max_;
}

bool ColorsFilterModel::IsSupportsColors() const {
  return supports_colors_;
}

bool ColorsFilterModel::IsSupportsTemperature() const {
  return supports_temperature_;
}

bool ColorsFilterModel::IsSupportsScenes() const {
  return supports_scenes_;
}

bool ColorsFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const {
  const auto index = sourceModel()->index(row, 0, parent);

  if (!index.isValid()) {
    return false;
  }

  const auto is_temperature = sourceModel()->data(
    index, ColorsModel::IsTemperatureRole
  ).toBool();

  if (is_temperature && supports_temperature_) {
    const auto temperature = sourceModel()->data(
      index, ColorsModel::TemperatureRole
    ).toInt();

    return (min_ <= temperature) && (temperature <= max_);
  }

  if (!is_temperature && supports_colors_) {
    return true;
  }

  return false;
}

bool ColorsFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
  const QVariant left_flag = sourceModel()->data(left, ColorsModel::IsTemperatureRole);
  const QVariant right_flag = sourceModel()->data(right, ColorsModel::IsTemperatureRole);

  int left_value = 0;
  int right_value = 0;

  if (left_flag.toBool() && right_flag.toBool()) {
    left_value = sourceModel()->data(left, ColorsModel::TemperatureRole).toInt();
    right_value = sourceModel()->data(right, ColorsModel::TemperatureRole).toInt();
  }

  return left_flag.toBool() && (left_value <= right_value);
}
