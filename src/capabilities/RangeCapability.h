#pragma once

#include <QObject>
#include <QMap>
#include <QVariant>

class RangeCapability : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
  Q_PROPERTY(int value READ GetValue WRITE SetValue NOTIFY valueChanged)
public:
  explicit RangeCapability(QObject *parent = nullptr);

  [[nodiscard]] QVariantMap GetState() const;
  [[nodiscard]] QVariantMap GetParameters() const;
  [[nodiscard]] int GetValue() const;

  Q_INVOKABLE QVariantMap Create(int value);

public slots:
  void SetState(const QVariantMap& state);
  void SetParameters(const QVariantMap& parameters);
  void SetValue(int value);

signals:
  void stateChanged();
  void parametersChanged();
  void valueChanged();

private:
  QVariantMap parameters_;
  QVariantMap state_;
};
