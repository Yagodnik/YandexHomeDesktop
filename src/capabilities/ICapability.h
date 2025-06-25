#pragma once

#include <QVariant>

class ICapability : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
  Q_PROPERTY(QVariant value READ GetValue WRITE SetValue NOTIFY valueChanged)
public:
  explicit ICapability(QObject *parent = nullptr);

  [[nodiscard]] QVariantMap GetState() const;
  [[nodiscard]] QVariantMap GetParameters() const;

  virtual void SetValue(const QVariant& value) = 0;

public slots:
  void SetState(const QVariantMap& state);
  void SetParameters(const QVariantMap& parameters);
  virtual QVariant GetValue() const = 0;

signals:
  void stateChanged();
  void parametersChanged();
  void valueChanged();

protected:
  QVariantMap parameters_;
  QVariantMap state_;
};
