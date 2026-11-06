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
#elif defined(Q_OS_LINUX)
#include <ctime>
#include <unistd.h>
#endif
#endif

namespace WorkerControlPrivate {
#ifdef Q_OS_WIN
static qint64 UnixMilliseconds(const FILETIME& time) {
  constexpr qint64 windows_epoch_ms = 11'644'473'600'000;
  const auto ticks = (quint64(time.dwHighDateTime) << 32) | time.dwLowDateTime;
  return qint64(ticks / 10'000) - windows_epoch_ms;
}
#endif

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
  return {QFileInfo(base).absolutePath(), base + ".lock", base + ".control.lock", base + ".launch",
          base + ".worker"};
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

bool ProcessIsRunning(qint64 pid, const QDateTime& observed) {
  if (pid <= 0) {
    return false;
  }
#ifdef Q_OS_WIN
  if (pid > std::numeric_limits<DWORD>::max()) {
    return false;
  }
  const auto process_id = static_cast<DWORD>(pid);
  auto process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
  if (!process && GetLastError() == ERROR_ACCESS_DENIED)
    process = OpenProcess(SYNCHRONIZE, FALSE, process_id);
  if (!process) {
    return GetLastError() != ERROR_INVALID_PARAMETER;
  }
  const auto state = WaitForSingleObject(process, 0);
  FILETIME creation, exit, kernel, user;
  const bool has_creation =
      observed.isValid() && GetProcessTimes(process, &creation, &exit, &kernel, &user);
  CloseHandle(process);
  if (has_creation && UnixMilliseconds(creation) > observed.toMSecsSinceEpoch())
    return false;
  return state != WAIT_OBJECT_0;
#else
  if (pid > std::numeric_limits<pid_t>::max()) {
    return false;
  }
  if (::kill(static_cast<pid_t>(pid), 0) != 0 && errno == ESRCH) {
    return false;
  }
  // Check process metadata even if kill returned EPERM. It also distinguishes
  // running children from zombies that their supervisors have yet to reap.
#ifdef Q_OS_MACOS
  int query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid)};
  kinfo_proc info{};
  size_t size = sizeof(info);
  if (sysctl(query, 4, &info, &size, nullptr, 0) == 0) {
    if (size == 0 || info.kp_proc.p_stat == SZOMB)
      return false;
    const auto start = info.kp_proc.p_starttime;
    return !observed.isValid() ||
           qint64(start.tv_sec) * 1'000 + start.tv_usec / 1'000 <= observed.toMSecsSinceEpoch();
  }
#elif defined(Q_OS_LINUX)
  QFile stat(QString("/proc/%1/stat").arg(pid));
  if (stat.open(QIODevice::ReadOnly)) {
    const auto bytes = stat.readAll();
    const auto end = bytes.lastIndexOf(')');
    if (end >= 0 && end + 2 < bytes.size()) {
      if (bytes[end + 2] == 'Z' || bytes[end + 2] == 'X')
        return false;
      // Field 22 is the process start in clock ticks since boot. Fields after
      // the parenthesized command name begin with field 3 (state).
      if (observed.isValid()) {
        bool valid = false;
        const auto ticks = bytes.mid(end + 2).split(' ').value(19).toLongLong(&valid);
        const auto ticks_per_second = sysconf(_SC_CLK_TCK);
        const auto now = QDateTime::currentMSecsSinceEpoch();
        timespec uptime{};
        if (valid && ticks_per_second > 0 && clock_gettime(CLOCK_BOOTTIME, &uptime) == 0) {
          const auto elapsed_ms = qint64(uptime.tv_sec) * 1'000 + uptime.tv_nsec / 1'000'000 -
                                  ticks * 1'000 / ticks_per_second;
          return now - elapsed_ms <= observed.toMSecsSinceEpoch();
        }
      }
    }
  }
#endif
  return true;
#endif
}

Receipt ReadReceipt(const QString& path) {
  QByteArray bytes;
  QDateTime written;
#ifdef Q_OS_WIN
  // Reading a receipt must not block an atomic replacement by QSaveFile.
  const auto file = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE)
    return {};
  char buffer[65];
  DWORD size = 0;
  FILETIME modified{};
  const bool read = ReadFile(file, buffer, sizeof(buffer), &size, nullptr);
  const bool has_time = GetFileTime(file, nullptr, nullptr, &modified);
  CloseHandle(file);
  if (!read || size >= sizeof(buffer))
    return {};
  bytes = QByteArray(buffer, size);
  if (has_time)
    written = QDateTime::fromMSecsSinceEpoch(UnixMilliseconds(modified));
#else
  QFile record(path);
  if (!record.open(QIODevice::ReadOnly) || record.size() > 64) {
    return {};
  }
  bytes = record.readAll();
  written = record.fileTime(QFileDevice::FileModificationTime);
#endif
  bool valid = false;
  const auto pid = bytes.trimmed().toLongLong(&valid);
  if (!valid || pid <= 0)
    return {};
  return {pid, written};
}

void ClearReceipts(const Paths& paths, qint64 pid) {
  // A replacement worker may already have written a new receipt.
  for (const auto& path : {paths.launch, paths.worker}) {
    const auto receipt = ReadReceipt(path);
    if (receipt.pid > 0 &&
        (pid > 0 ? receipt.pid == pid : !ProcessIsRunning(receipt.pid, receipt.written)))
      QFile::remove(path);
  }
}

QJsonObject Error(const QString& code) {
  // Presentation adapters translate errors; the transport has no UI strings.
  return {{"ok", false}, {"error", QJsonObject{{"code", code}}}};
}
} // namespace WorkerControlPrivate
