#pragma once

#include <QTimer>
#include <memory>
#include "CliCommand.h"
#include "CliContext.h"

// Execution mechanics only. ICommand implementations own business operations.
class CliRunner final : public QObject, public CliContext {
  Q_OBJECT
public:
  using Writer = std::function<void(const QByteArray& text, bool stderr_stream)>;
  explicit CliRunner(CliServices services, Writer writer, QObject* parent = nullptr);
  void Start(const CliCommand& command);
  static QByteArray FormatError(bool json, const QString& code, const QString& message,
                               const QJsonObject& details = {});
  QObject* Owner() const override;
  const CliServices& Services() const override;
  void Complete(const QJsonObject& output, const QString& text) override;
  void FailApi(const ApiError& error) override;
  void Fail(int exit_code, const QString& code, const QString& message, const QJsonObject& details = {}) override;

signals:
  void finished(int exit_code);
private slots:
  void OnTimeout();
private:
  void Finish(int exit_code, const QByteArray& output);
  CliServices services_;
  Writer writer_;
  CliCommand command_;
  QTimer timer_;
  std::unique_ptr<QObject> operation_;
};
