#pragma once

#include "CliCommand.h"
#include <QTimer>
#include <functional>

// Transient terminal feedback, cleared before printing command results.
class CliProgress final : public QObject {
  Q_OBJECT
public:
  using Writer = std::function<void(const QByteArray&)>;

  CliProgress(bool enabled, Writer writer, QObject* parent = nullptr);
  ~CliProgress() override;
  static bool EnabledFor(const CliCommand& command, bool terminal);
  void SetMessage(const QString& message);
  void Stop();

private slots:
  void Advance();

private:
  void Render();

  bool enabled_;
  Writer writer_;
  QTimer timer_;
  QString message_;
  int frame_ = 0;
  int columns_ = 0;
};
