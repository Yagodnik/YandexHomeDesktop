#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>

namespace WorkerControlPrivate {
constexpr int RetryIntervalMs = 25;
constexpr qsizetype MaxRequestBytes = 64 * 1024;
constexpr qsizetype MaxResponseBytes = 64 * 1024;

struct Paths {
  QString directory;
  QString ownership;
  QString transition;
  QString launch;
  QString worker;
};

struct Receipt {
  qint64 pid = 0;
  QDateTime observed;
};

Paths ControlPaths(const QString& name);
bool PrepareDirectory(const Paths& paths);
QDateTime ObserveProcess(qint64 pid);
bool ProcessIsRunning(qint64 pid, const QDateTime& observed);
QByteArray EncodeReceipt(const Receipt& receipt);
Receipt ReadReceipt(const QString& path);
// If no PID is supplied, remove only receipts for inactive processes.
void ClearReceipts(const Paths& paths, qint64 pid = 0);
QJsonObject Error(const QString& code);
} // namespace WorkerControlPrivate
