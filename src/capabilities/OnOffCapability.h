#pragma once

#include <QMap>
#include <QObject>

class OnOffCapability : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap state READ GetState WRITE SetState NOTIFY stateChanged)
  Q_PROPERTY(QVariantMap parameters READ GetParameters WRITE SetParameters NOTIFY parametersChanged)
  Q_PROPERTY(bool value READ GetValue WRITE SetValue NOTIFY valueChanged)
public:
  explicit OnOffCapability(QObject *parent = nullptr);

  [[nodiscard]] QVariantMap GetState() const;
  [[nodiscard]] QVariantMap GetParameters() const;
  [[nodiscard]] bool GetValue() const;

  Q_INVOKABLE QVariantMap Create(bool value);

public slots:
  void SetState(const QVariantMap& state);
  void SetParameters(const QVariantMap& parameters);
  void SetValue(bool value);

signals:
  void stateChanged();
  void parametersChanged();
  void valueChanged();

private:
  QVariantMap parameters_;
  QVariantMap state_;
};
