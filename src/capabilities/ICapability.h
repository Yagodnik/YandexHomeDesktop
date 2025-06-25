#pragma once

#include <QVariant>

class ICapability : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
public:
  explicit ICapability(QObject *parent = nullptr);

  [[nodiscard]] QVariantMap GetState() const;
  [[nodiscard]] QVariantMap GetParameters() const;

public slots:
  void SetState(const QVariantMap& state);
  void SetParameters(const QVariantMap& parameters);

signals:
  void stateChanged();
  void parametersChanged();

protected:
  QVariantMap parameters_;
  QVariantMap state_;
};
