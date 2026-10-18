#include "WorkerControlPrivate.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <limits>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <cerrno>
#include <csignal>
#ifdef Q_OS_MACOS
#include <sys/proc.h>
#include <sys/sysctl.h>
#endif
#endif

namespace WorkerControlPrivate {
Paths ControlPaths(const QString& name) {
  QString base;
#ifdef Q_OS_UNIX
  if (QFileInfo(name).isAbsolute()) {
    base = name;
  } else
#endif
  {
    // Named pipes and short socket names still need private filesystem locks.
    const auto digest =
        QString::fromLatin1(QCryptographicHash::hash((QDir::homePath() + '/' + name).toUtf8(),
                                                     QCryptographicHash::Sha256)
                                .toHex());
    base = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
               .filePath("YandexHome/run/" + digest);
  }
  return {QFileInfo(base).absolutePath(), base + ".lock", base + ".control.lock", base + ".launch"};
}

bool PrepareDirectory(const Paths& paths) {
  if (!QDir().mkpath(paths.directory)) {
    return false;
  }
#ifdef Q_OS_UNIX
  return QFile::setPermissions(paths.directory, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                                    QFileDevice::ExeOwner);
#else
  return true;
#endif
}

bool ProcessIsRunning(qint64 pid) {
  if (pid <= 0) {
    return false;
  }
#ifdef Q_OS_WIN
  if (pid > std::numeric_limits<DWORD>::max()) {
    return false;
  }
  const auto process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
  if (!process) {
    return GetLastError() != ERROR_INVALID_PARAMETER;
  }
  const auto state = WaitForSingleObject(process, 0);
  CloseHandle(process);
  return state != WAIT_OBJECT_0;
#else
  if (pid > std::numeric_limits<pid_t>::max()) {
    return false;
  }
  if (::kill(static_cast<pid_t>(pid), 0) != 0) {
    return errno != ESRCH;
  }
  // A child can have exited without its supervisor having reaped it yet.
#ifdef Q_OS_MACOS
  int query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid)};
  kinfo_proc info{};
  size_t size = sizeof(info);
  if (sysctl(query, 4, &info, &size, nullptr, 0) == 0) {
    return size != 0 && info.kp_proc.p_stat != SZOMB;
  }
#elif defined(Q_OS_LINUX)
  QFile stat(QString("/proc/%1/stat").arg(pid));
  if (stat.open(QIODevice::ReadOnly)) {
    const auto bytes = stat.readAll();
    const auto end = bytes.lastIndexOf(')');
    if (end >= 0 && end + 2 < bytes.size()) {
      return bytes[end + 2] != 'Z' && bytes[end + 2] != 'X';
    }
  }
#endif
  return true;
#endif
}

qint64 LaunchPid(const Paths& paths) {
  QFile record(paths.launch);
  if (!record.open(QIODevice::ReadOnly) || record.size() > 64) {
    return 0;
  }
  bool valid = false;
  const auto pid = record.readAll().trimmed().toLongLong(&valid);
  return valid && pid > 0 ? pid : 0;
}

void ClearLaunch(const Paths& paths, qint64 pid) {
  // A replacement worker may already have written a new receipt.
  if (pid > 0 && LaunchPid(paths) == pid) {
    QFile::remove(paths.launch);
  }
}

QJsonObject Error(const QString& code) {
  // Presentation adapters translate errors; the transport has no UI strings.
  return {{"ok", false}, {"error", QJsonObject{{"code", code}}}};
}
} // namespace WorkerControlPrivate
