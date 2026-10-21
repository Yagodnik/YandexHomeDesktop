#pragma once

#include "yh/workercontrol_export.h"

#include <QJsonObject>
#include <QLocalServer>
#include <QLockFile>
#include <QObject>
#include <functional>
#include <memory>

// One worker owns the endpoint until destruction, including its cleanup period.
// This class does not decide when an application starts or exits.
class WORKERCONTROL_EXPORT WorkerControlServer final : public QObject {
  Q_OBJECT
public:
  using Handler = std::function<QJsonObject(const QJsonObject&)>;
  explicit WorkerControlServer(QObject* parent = nullptr);
  ~WorkerControlServer() override;

  bool Acquire(const QString& control_name);
  bool Listen();
  bool Start(const QString& control_name);
  // Stop accepting requests while retaining process ownership until destruction.
  void Close();
  void SetHandler(Handler handler);
  QString ErrorCode() const;

private:
  void AcceptConnections();
  QLocalServer server_;
  std::unique_ptr<QLockFile> ownership_;
  QString name_;
  QString error_;
  Handler handler_;
};
