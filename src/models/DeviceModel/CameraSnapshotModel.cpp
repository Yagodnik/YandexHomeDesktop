#include "CameraSnapshotModel.h"

#include <QDateTime>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QVideoFrame>
#include <QVideoSink>

bool CameraSnapshotModel::Capture(QObject* video_sink) {
  const auto* sink = qobject_cast<QVideoSink*>(video_sink);
  frame_ = sink ? sink->videoFrame().toImage() : QImage{};
  status_ = frame_.isNull() ? tr("Не удалось сделать снимок") : QString{};
  const auto name =
      "Camera-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss-zzz") + ".png";
  suggested_file_ = QUrl::fromLocalFile(
      QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).filePath(name));
  emit changed();
  return !frame_.isNull();
}

bool CameraSnapshotModel::Save(const QUrl& destination) {
  bool saved = false;
  if (!frame_.isNull() && destination.isLocalFile()) {
    QSaveFile file(destination.toLocalFile());
    saved = file.open(QIODevice::WriteOnly) && frame_.save(&file, "PNG") && file.commit();
  }
  status_ = saved ? tr("Снимок сохранён") : tr("Не удалось сохранить снимок");
  emit changed();
  return saved;
}

void CameraSnapshotModel::Clear() {
  frame_ = {};
  status_.clear();
  suggested_file_.clear();
  emit changed();
}
