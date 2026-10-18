#pragma once

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
};

Paths ControlPaths(const QString& name);
bool PrepareDirectory(const Paths& paths);
bool ProcessIsRunning(qint64 pid);
qint64 LaunchPid(const Paths& paths);
void ClearLaunch(const Paths& paths, qint64 pid);
QJsonObject Error(const QString& code);
} // namespace WorkerControlPrivate
