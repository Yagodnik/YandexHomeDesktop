#pragma once

#include <QSortFilterProxyModel>

class ColorsFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(int min READ GetMin WRITE SetMin NOTIFY minChanged)
  Q_PROPERTY(int max READ GetMax WRITE SetMax NOTIFY maxChanged)
  Q_PROPERTY(bool supportsColors READ IsSupportsColors WRITE SetSupportsColors NOTIFY supportsColorsChanged)
  Q_PROPERTY(bool supportsTemperatures READ IsSupportsTemperature WRITE SetSupportsTemperature NOTIFY supportsTemperatureChanged)
  Q_PROPERTY(bool supportsScenes READ IsSupportsScenes WRITE SetSupportsScenes NOTIFY supportsScenesChanged)
public:
  explicit ColorsFilterModel(QObject *parent = nullptr);

  void SetMin(int min);
  void SetMax(int max);
  void SetSupportsColors(bool value);
  void SetSupportsTemperature(bool value);
  void SetSupportsScenes(bool value);

public slots:
  [[nodiscard]] int GetMin() const;
  [[nodiscard]] int GetMax() const;
  [[nodiscard]] bool IsSupportsColors() const;
  [[nodiscard]] bool IsSupportsTemperature() const;
  [[nodiscard]] bool IsSupportsScenes() const;

signals:
  void minChanged();
  void maxChanged();
  void supportsColorsChanged();
  void supportsTemperatureChanged();
  void supportsScenesChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
  [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

  int min_;
  int max_;
  bool supports_colors_{};
  bool supports_temperature_{};
  bool supports_scenes_{};
};

